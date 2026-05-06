import { createServer } from "node:http";
import { connect } from "node:http2";
import { readFile } from "node:fs/promises";
import { extname, join, normalize } from "node:path";
import { fileURLToPath } from "node:url";

const ROOT = fileURLToPath(new URL(".", import.meta.url));
const PORT = Number.parseInt(process.env.OTHELLO_WEB_PORT ?? "8080", 10);
const GRPC_TARGET = process.env.OTHELLO_GRPC_TARGET ?? "http://localhost:50051";

const CONTENT_TYPES = new Map([
  [".html", "text/html; charset=utf-8"],
  [".css", "text/css; charset=utf-8"],
  [".js", "text/javascript; charset=utf-8"],
  [".json", "application/json; charset=utf-8"],
]);

createServer(async (request, response) => {
  try {
    if (request.method === "POST" && request.url === "/api/find-best-move") {
      const payload = JSON.parse(await readBody(request));
      const result = await findBestMove(payload);
      writeJson(response, 200, result);
      return;
    }

    if (request.method !== "GET") {
      writeJson(response, 405, { error: "Method not allowed" });
      return;
    }

    await serveStatic(request, response);
  } catch (error) {
    writeJson(response, 500, {
      error: error instanceof Error ? error.message : "Unexpected server error",
    });
  }
}).listen(PORT, () => {
  console.log(`Othello frontend listening on http://localhost:${PORT}`);
  console.log(`Proxying engine requests to ${GRPC_TARGET}`);
});

function readBody(request) {
  return new Promise((resolve, reject) => {
    let body = "";
    request.setEncoding("utf8");
    request.on("data", (chunk) => {
      body += chunk;
      if (body.length > 64_000) {
        request.destroy(new Error("Request body too large"));
      }
    });
    request.on("end", () => resolve(body));
    request.on("error", reject);
  });
}

async function serveStatic(request, response) {
  const url = new URL(request.url ?? "/", `http://${request.headers.host ?? "localhost"}`);
  const pathname = url.pathname === "/" ? "/index.html" : url.pathname;
  const relativePath = normalize(decodeURIComponent(pathname)).replace(/^(\.\.[/\\])+/, "");
  const filePath = join(ROOT, relativePath);

  if (!filePath.startsWith(ROOT)) {
    writeJson(response, 403, { error: "Forbidden" });
    return;
  }

  try {
    const content = await readFile(filePath);
    response.writeHead(200, {
      "content-type": CONTENT_TYPES.get(extname(filePath)) ?? "application/octet-stream",
      "cache-control": "no-store",
    });
    response.end(content);
  } catch {
    writeJson(response, 404, { error: "Not found" });
  }
}

function findBestMove(payload) {
  const requestMessage = encodeFindBestMoveRequest(payload);
  const grpcFrame = Buffer.alloc(5 + requestMessage.length);
  grpcFrame[0] = 0;
  grpcFrame.writeUInt32BE(requestMessage.length, 1);
  requestMessage.copy(grpcFrame, 5);

  return new Promise((resolve, reject) => {
    const client = connect(GRPC_TARGET);
    const chunks = [];
    let grpcStatus = "0";
    let grpcMessage = "";

    client.on("error", reject);

    const stream = client.request({
      ":method": "POST",
      ":path": "/engine.EngineService/FindBestMove",
      "content-type": "application/grpc",
      te: "trailers",
    });

    stream.on("response", (headers) => {
      const status = headers[":status"];
      grpcStatus = String(headers["grpc-status"] ?? grpcStatus);
      grpcMessage = decodeURIComponent(String(headers["grpc-message"] ?? grpcMessage));
      if (status !== 200) {
        reject(new Error(`gRPC HTTP status ${status}`));
      }
    });

    stream.on("data", (chunk) => chunks.push(chunk));
    stream.on("trailers", (headers) => {
      grpcStatus = String(headers["grpc-status"] ?? grpcStatus);
      grpcMessage = decodeURIComponent(String(headers["grpc-message"] ?? ""));
    });
    stream.on("error", reject);
    stream.on("end", () => {
      client.close();
      const responseFrame = Buffer.concat(chunks);
      if (grpcStatus !== "0") {
        reject(new Error(`gRPC status ${grpcStatus}${grpcMessage ? `: ${grpcMessage}` : ""}`));
        return;
      }
      if (responseFrame.length < 5) {
        reject(new Error("Empty gRPC response"));
        return;
      }
      const compressed = responseFrame[0];
      const length = responseFrame.readUInt32BE(1);
      if (compressed !== 0) {
        reject(new Error("Compressed gRPC responses are not supported"));
        return;
      }
      resolve(decodeFindBestMoveResponse(responseFrame.subarray(5, 5 + length)));
    });

    stream.end(grpcFrame);
  });
}

function encodeFindBestMoveRequest(payload) {
  const gameState = payload?.game_state ?? {};
  return Buffer.concat([
    encodeField(1, 2, encodeGameState(gameState)),
    encodeField(2, 0, encodeVarint(BigInt(payload?.time_limit_ms ?? 0))),
    encodeField(3, 0, encodeVarint(BigInt(payload?.depth_limit ?? 0))),
  ]);
}

function encodeGameState(gameState) {
  return Buffer.concat([
    encodeField(1, 0, encodeVarint(BigInt(gameState.black_bb ?? 0))),
    encodeField(2, 0, encodeVarint(BigInt(gameState.white_bb ?? 0))),
    encodeField(3, 0, encodeVarint(gameState.black_to_move ? 1n : 0n)),
  ]);
}

function encodeField(fieldNumber, wireType, payload) {
  const key = encodeVarint(BigInt((fieldNumber << 3) | wireType));
  if (wireType === 2) {
    return Buffer.concat([key, encodeVarint(BigInt(payload.length)), payload]);
  }
  return Buffer.concat([key, payload]);
}

function encodeVarint(value) {
  const bytes = [];
  let current = value;
  do {
    let byte = Number(current & 0x7fn);
    current >>= 7n;
    if (current !== 0n) {
      byte |= 0x80;
    }
    bytes.push(byte);
  } while (current !== 0n);
  return Buffer.from(bytes);
}

function decodeFindBestMoveResponse(buffer) {
  let offset = 0;
  const result = { best_move: -1, eval_score: 0 };

  while (offset < buffer.length) {
    const tag = readVarint(buffer, offset);
    offset = tag.offset;
    const fieldNumber = Number(tag.value >> 3n);
    const wireType = Number(tag.value & 0x07n);
    if (wireType !== 0) {
      throw new Error(`Unsupported response wire type ${wireType}`);
    }
    const value = readVarint(buffer, offset);
    offset = value.offset;

    if (fieldNumber === 1) {
      result.best_move = toSignedInt32(value.value);
    } else if (fieldNumber === 2) {
      result.eval_score = toSignedInt32(value.value);
    }
  }

  return result;
}

function readVarint(buffer, start) {
  let shift = 0n;
  let value = 0n;
  let offset = start;

  while (offset < buffer.length) {
    const byte = buffer[offset];
    value |= BigInt(byte & 0x7f) << shift;
    offset += 1;
    if ((byte & 0x80) === 0) {
      return { value, offset };
    }
    shift += 7n;
  }

  throw new Error("Truncated varint");
}

function toSignedInt32(value) {
  const unsigned = Number(value & 0xffffffffn);
  return unsigned >= 0x80000000 ? unsigned - 0x100000000 : unsigned;
}

function writeJson(response, status, payload) {
  response.writeHead(status, { "content-type": "application/json; charset=utf-8" });
  response.end(JSON.stringify(payload));
}
