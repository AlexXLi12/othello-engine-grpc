const STORAGE_KEY = "othello-engine-console";
const BOARD_SIZE = 64;
const FULL_MASK = (1n << 64n) - 1n;
const LEFT_EDGE_MASK = BigInt.asUintN(64, ~0x0101010101010101n);
const RIGHT_EDGE_MASK = BigInt.asUintN(64, ~0x8080808080808080n);
const TOP_EDGE_MASK = BigInt.asUintN(64, ~0x00000000000000ffn);
const BOTTOM_EDGE_MASK = BigInt.asUintN(64, ~0xff00000000000000n);
const DIRECTIONS = [
  { shift: -1, mask: LEFT_EDGE_MASK },
  { shift: 1, mask: RIGHT_EDGE_MASK },
  { shift: 8, mask: BOTTOM_EDGE_MASK },
  { shift: -8, mask: TOP_EDGE_MASK },
  { shift: -7, mask: TOP_EDGE_MASK & RIGHT_EDGE_MASK },
  { shift: -9, mask: TOP_EDGE_MASK & LEFT_EDGE_MASK },
  { shift: 7, mask: BOTTOM_EDGE_MASK & LEFT_EDGE_MASK },
  { shift: 9, mask: BOTTOM_EDGE_MASK & RIGHT_EDGE_MASK },
];

const STARTING_BOARD = Array.from({ length: BOARD_SIZE }, () => null);
STARTING_BOARD[27] = "white";
STARTING_BOARD[28] = "black";
STARTING_BOARD[35] = "black";
STARTING_BOARD[36] = "white";

const DEFAULT_STATE = {
  board: STARTING_BOARD,
  turn: "black",
  paint: "black",
  editMode: false,
  depthLimit: 8,
  timeLimitMs: 1000,
  showRawResponse: false,
  lastResponse: null,
};

const elements = {
  board: document.querySelector("#board"),
  blackCount: document.querySelector("#blackCount"),
  whiteCount: document.querySelector("#whiteCount"),
  emptyCount: document.querySelector("#emptyCount"),
  turnBadge: document.querySelector("#turnBadge"),
  paintBadge: document.querySelector("#paintBadge"),
  gameStatus: document.querySelector("#gameStatus"),
  serverStatus: document.querySelector("#serverStatus"),
  turnBlack: document.querySelector("#turnBlack"),
  turnWhite: document.querySelector("#turnWhite"),
  paintBlack: document.querySelector("#paintBlack"),
  paintWhite: document.querySelector("#paintWhite"),
  paintEmpty: document.querySelector("#paintEmpty"),
  editMode: document.querySelector("#editMode"),
  depthLimit: document.querySelector("#depthLimit"),
  timeLimitMs: document.querySelector("#timeLimitMs"),
  engineMove: document.querySelector("#engineMove"),
  resetBoard: document.querySelector("#resetBoard"),
  clearBoard: document.querySelector("#clearBoard"),
  copyRequest: document.querySelector("#copyRequest"),
  requestPreview: document.querySelector("#requestPreview"),
  evalSummary: document.querySelector("#evalSummary"),
  responsePreview: document.querySelector("#responsePreview"),
  toggleRawResponse: document.querySelector("#toggleRawResponse"),
};

let state = loadState();
let engineRequestInFlight = false;

function loadState() {
  const raw = sessionStorage.getItem(STORAGE_KEY);
  if (!raw) {
    return structuredClone(DEFAULT_STATE);
  }

  try {
    const parsed = JSON.parse(raw);
    return {
      ...structuredClone(DEFAULT_STATE),
      ...parsed,
      board: normalizeBoard(parsed.board),
    };
  } catch {
    return structuredClone(DEFAULT_STATE);
  }
}

function normalizeBoard(board) {
  if (!Array.isArray(board) || board.length !== BOARD_SIZE) {
    return [...STARTING_BOARD];
  }
  return board.map((value) => (value === "black" || value === "white" ? value : null));
}

function saveState() {
  sessionStorage.setItem(STORAGE_KEY, JSON.stringify(state));
}

function bitAt(index) {
  return 1n << BigInt(index);
}

function bitboardFor(piece) {
  let value = 0n;
  state.board.forEach((cell, index) => {
    if (cell === piece) {
      value |= bitAt(index);
    }
  });
  return value;
}

function shiftBits(value, shift, mask) {
  const masked = value & mask;
  if (shift > 0) {
    return BigInt.asUintN(64, masked << BigInt(shift));
  }
  return BigInt.asUintN(64, masked >> BigInt(-shift));
}

function boardsFor(board, color) {
  const black = boardToBitboard(board, "black");
  const white = boardToBitboard(board, "white");
  return color === "black" ? { mine: black, theirs: white } : { mine: white, theirs: black };
}

function boardToBitboard(board, piece) {
  let value = 0n;
  board.forEach((cell, index) => {
    if (cell === piece) {
      value |= bitAt(index);
    }
  });
  return value;
}

function directionalMoves(mine, theirs, empty, shift, mask) {
  let run = shiftBits(mine, shift, mask) & theirs;
  for (let i = 0; i < 5; i += 1) {
    run |= shiftBits(run, shift, mask) & theirs;
  }
  return shiftBits(run, shift, mask) & empty;
}

function possibleMoves(board, color) {
  const { mine, theirs } = boardsFor(board, color);
  const empty = FULL_MASK & ~(mine | theirs);
  return DIRECTIONS.reduce(
    (moves, direction) =>
      moves | directionalMoves(mine, theirs, empty, direction.shift, direction.mask),
    0n,
  );
}

function directionalFlips(move, mine, theirs, shift, mask) {
  let cursor = move;
  let flips = 0n;
  while (true) {
    cursor = shiftBits(cursor, shift, mask);
    if (cursor & theirs) {
      flips |= cursor;
    } else if (cursor & mine) {
      return flips;
    } else {
      return 0n;
    }
  }
}

function flipsForMove(board, index, color) {
  const move = bitAt(index);
  const { mine, theirs } = boardsFor(board, color);
  return DIRECTIONS.reduce(
    (flips, direction) =>
      flips | directionalFlips(move, mine, theirs, direction.shift, direction.mask),
    0n,
  );
}

function isLegalMove(board, index, color) {
  if (board[index] !== null) {
    return false;
  }
  return Boolean(possibleMoves(board, color) & bitAt(index));
}

function applyMove(board, index, color) {
  const next = [...board];
  const flips = flipsForMove(board, index, color);
  next[index] = color;
  for (let i = 0; i < BOARD_SIZE; i += 1) {
    if (flips & bitAt(i)) {
      next[i] = color;
    }
  }
  return next;
}

function opponent(color) {
  return color === "black" ? "white" : "black";
}

function advanceTurn(board, color) {
  const other = opponent(color);
  if (possibleMoves(board, other) !== 0n) {
    return other;
  }
  return color;
}

function normalizeTurnForAvailableMoves(board, color) {
  if (isTerminal(board) || possibleMoves(board, color) !== 0n) {
    return color;
  }
  return opponent(color);
}

function isTerminal(board) {
  return possibleMoves(board, "black") === 0n && possibleMoves(board, "white") === 0n;
}

function requestPayload() {
  return {
    time_limit_ms: Number(state.timeLimitMs),
    depth_limit: Number(state.depthLimit),
    game_state: {
      black_bb: bitboardFor("black").toString(),
      white_bb: bitboardFor("white").toString(),
      black_to_move: state.turn === "black",
    },
  };
}

function renderBoard() {
  elements.board.replaceChildren();
  const legalMoves = possibleMoves(state.board, state.turn);

  state.board.forEach((cell, index) => {
    const square = document.createElement("button");
    const legal = Boolean(legalMoves & bitAt(index));
    square.type = "button";
    square.className = `square${legal ? " legal" : ""}`;
    square.setAttribute("role", "gridcell");
    square.setAttribute("aria-label", `${squareName(index)} ${cell ?? (legal ? "legal move" : "empty")}`);
    square.dataset.index = String(index);

    if (cell) {
      const disc = document.createElement("span");
      disc.className = `disc ${cell}`;
      square.append(disc);
    }

    square.addEventListener("click", () => {
      if (isLegalMove(state.board, index, state.turn)) {
        applyLocalMove(index);
      } else if (state.editMode) {
        state.board[index] = state.paint === "empty" ? null : state.paint;
        const requestedTurn = state.turn;
        state.turn = normalizeTurnForAvailableMoves(state.board, state.turn);
        if (state.turn !== requestedTurn) {
          setServerStatus(`${label(requestedTurn)} has no legal move. Passing to ${label(state.turn)}.`);
        } else {
          setServerStatus("Position edited.");
        }
        commit();
      } else {
        setServerStatus(`${squareName(index)} is not legal for ${label(state.turn)}.`, "error");
      }
    });

    elements.board.append(square);
  });
}

function squareName(index) {
  const file = String.fromCharCode(65 + (index % 8));
  const rank = Math.floor(index / 8) + 1;
  return `${file}${rank}`;
}

function renderControls() {
  const blackCount = state.board.filter((cell) => cell === "black").length;
  const whiteCount = state.board.filter((cell) => cell === "white").length;
  const moves = possibleMoves(state.board, state.turn);

  elements.blackCount.textContent = String(blackCount);
  elements.whiteCount.textContent = String(whiteCount);
  elements.emptyCount.textContent = String(BOARD_SIZE - blackCount - whiteCount);
  elements.turnBadge.textContent = label(state.turn);
  elements.paintBadge.textContent = label(state.paint);
  elements.gameStatus.textContent = gameStatusText(blackCount, whiteCount, moves);
  elements.engineMove.disabled = engineRequestInFlight || moves === 0n || isTerminal(state.board);
  elements.depthLimit.value = String(state.depthLimit);
  elements.timeLimitMs.value = String(state.timeLimitMs);

  setSelected(elements.turnBlack, state.turn === "black");
  setSelected(elements.turnWhite, state.turn === "white");
  setSelected(elements.paintBlack, state.paint === "black");
  setSelected(elements.paintWhite, state.paint === "white");
  setSelected(elements.paintEmpty, state.paint === "empty");
  setSelected(elements.editMode, state.editMode);
  elements.editMode.textContent = state.editMode ? "Editing board" : "Edit board";

  elements.requestPreview.textContent = JSON.stringify(requestPayload(), null, 2);
  renderResponse();
}

function gameStatusText(blackCount, whiteCount, moves) {
  if (isTerminal(state.board)) {
    if (blackCount === whiteCount) {
      return "Draw";
    }
    return `${blackCount > whiteCount ? "Black" : "White"} wins`;
  }
  if (moves === 0n) {
    return `${label(state.turn)} has no move`;
  }
  return `${label(state.turn)} to move`;
}

function label(value) {
  return value[0].toUpperCase() + value.slice(1);
}

function setSelected(element, selected) {
  element.classList.toggle("selected", selected);
  element.setAttribute("aria-pressed", String(selected));
}

function setServerStatus(message, kind = "") {
  elements.serverStatus.textContent = message;
  elements.serverStatus.className = `server-status ${kind}`.trim();
}

function renderResponse() {
  if (!state.lastResponse) {
    elements.evalSummary.textContent = "No engine response yet.";
    elements.responsePreview.textContent = "";
  } else {
    elements.evalSummary.textContent = evalSummary(state.lastResponse);
    elements.responsePreview.textContent = JSON.stringify(state.lastResponse, null, 2);
  }

  elements.responsePreview.classList.toggle("hidden", !state.showRawResponse);
  elements.toggleRawResponse.textContent = state.showRawResponse ? "Hide raw" : "Raw";
}

function evalSummary(response) {
  const score = Number(response.eval_score ?? 0);
  const absoluteScore = Math.abs(score);

  if (score === 0) {
    return "Evaluation is even.";
  }

  const side = score > 0 ? "Black" : "White";
  return `${side} favored by ${absoluteScore}.`;
}

function commit() {
  saveState();
  renderBoard();
  renderControls();
}

function setTurn(turn) {
  state.turn = normalizeTurnForAvailableMoves(state.board, turn);
  if (state.turn !== turn) {
    setServerStatus(`${label(turn)} has no legal move. Passing to ${label(state.turn)}.`);
  }
  commit();
}

function setPaint(paint) {
  state.paint = paint;
  commit();
}

function clampNumber(value, min, max, fallback) {
  const parsed = Number.parseInt(value, 10);
  if (!Number.isFinite(parsed)) {
    return fallback;
  }
  return Math.min(Math.max(parsed, min), max);
}

function applyLocalMove(index) {
  const color = state.turn;
  state.board = applyMove(state.board, index, color);
  const nextTurn = advanceTurn(state.board, color);
  state.turn = normalizeTurnForAvailableMoves(state.board, nextTurn);
  const passText = state.turn === color ? ` ${label(opponent(color))} has no legal move.` : "";
  setServerStatus(`${label(color)} played ${squareName(index)}.${passText}`);
  commit();

  if (!isTerminal(state.board) && possibleMoves(state.board, state.turn) !== 0n) {
    window.setTimeout(() => {
      requestEngineMove();
    }, 0);
  }
}

async function requestEngineMove() {
  if (engineRequestInFlight) {
    return;
  }
  if (isTerminal(state.board) || possibleMoves(state.board, state.turn) === 0n) {
    return;
  }

  engineRequestInFlight = true;
  elements.engineMove.disabled = true;
  setServerStatus("Requesting engine move...");

  try {
    const response = await fetch("/api/find-best-move", {
      method: "POST",
      headers: { "content-type": "application/json" },
      body: JSON.stringify(requestPayload()),
    });
    const payload = await response.json();
    if (!response.ok) {
      throw new Error(payload.error ?? "Engine request failed");
    }
    state.lastResponse = payload;
    if (payload.best_move >= 0) {
      state.board = applyMove(state.board, payload.best_move, state.turn);
      state.turn = normalizeTurnForAvailableMoves(
        state.board,
        advanceTurn(state.board, state.turn),
      );
      setServerStatus(`Engine played ${squareName(payload.best_move)}. ${evalSummary(payload)}`, "ok");
    } else {
      const passed = state.turn;
      state.turn = normalizeTurnForAvailableMoves(state.board, opponent(state.turn));
      setServerStatus(`Engine passed. ${label(state.turn)} to move.`, passed === state.turn ? "error" : "ok");
    }
    commit();
  } catch (error) {
    setServerStatus(error instanceof Error ? error.message : "Engine request failed", "error");
    renderControls();
  } finally {
    engineRequestInFlight = false;
    renderControls();
  }
}

elements.turnBlack.addEventListener("click", () => setTurn("black"));
elements.turnWhite.addEventListener("click", () => setTurn("white"));
elements.paintBlack.addEventListener("click", () => setPaint("black"));
elements.paintWhite.addEventListener("click", () => setPaint("white"));
elements.paintEmpty.addEventListener("click", () => setPaint("empty"));
elements.editMode.addEventListener("click", () => {
  state.editMode = !state.editMode;
  setServerStatus(state.editMode ? "Board editing enabled." : "Board editing disabled.");
  commit();
});
elements.engineMove.addEventListener("click", requestEngineMove);
elements.toggleRawResponse.addEventListener("click", () => {
  state.showRawResponse = !state.showRawResponse;
  commit();
});

elements.depthLimit.addEventListener("change", (event) => {
  state.depthLimit = clampNumber(event.target.value, 1, 60, DEFAULT_STATE.depthLimit);
  commit();
});

elements.timeLimitMs.addEventListener("change", (event) => {
  state.timeLimitMs = clampNumber(event.target.value, 1, 3_600_000, DEFAULT_STATE.timeLimitMs);
  commit();
});

elements.resetBoard.addEventListener("click", () => {
  state.board = [...STARTING_BOARD];
  state.turn = "black";
  setServerStatus("Starting board restored.");
  commit();
});

elements.clearBoard.addEventListener("click", () => {
  state.board = Array.from({ length: BOARD_SIZE }, () => null);
  state.turn = "black";
  setServerStatus("Board cleared.");
  commit();
});

elements.copyRequest.addEventListener("click", async () => {
  const text = JSON.stringify(requestPayload(), null, 2);
  if (!navigator.clipboard) {
    elements.requestPreview.focus();
    return;
  }

  try {
    await navigator.clipboard.writeText(text);
  } catch {
    elements.requestPreview.focus();
    return;
  }

  elements.copyRequest.textContent = "Copied";
  window.setTimeout(() => {
    elements.copyRequest.textContent = "Copy";
  }, 900);
});

commit();
