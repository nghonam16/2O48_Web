let matrix = [
  [0, 0, 0, 0],
  [0, 0, 0, 0],
  [0, 0, 0, 0],
  [0, 0, 0, 0]
];
let score = 0;
let undoStack = [];
let redoStack = [];
const username = localStorage.getItem("username");

if (!username) {
  alert("Bạn chưa đăng nhập! Quay lại trang đăng nhập.");
  window.location.href = "/login.html";
}

document.addEventListener("DOMContentLoaded", () => {
  document.getElementById("usernameDisplay").textContent = username;
  loadGame();
});

function renderGrid() {
  const grid = document.getElementById("grid");
  grid.innerHTML = "";
  matrix.forEach(row => {
    row.forEach(cell => {
      const div = document.createElement("div");
      div.className = "cell";
      if (cell !== 0) {
        div.classList.add(`cell-${cell}`);
        div.textContent = cell;
      }
      grid.appendChild(div);
    });
  });
  document.getElementById("score").textContent = score;
}

function rotateMatrix(mat) {
  return mat[0].map((_, i) => mat.map(row => row[i]).reverse());
}

function slide(row) {
  let arr = row.filter(v => v !== 0);
  for (let i = 0; i < arr.length - 1; i++) {
    if (arr[i] === arr[i + 1]) {
      arr[i] *= 2;
      score += arr[i];
      arr[i + 1] = 0;
    }
  }
  return [...arr.filter(v => v !== 0), ...Array(4 - arr.filter(v => v !== 0).length).fill(0)];
}

function pushUndo() {
  undoStack.push({
    matrix: JSON.parse(JSON.stringify(matrix)),
    score: score
  });
  redoStack = [];
}

function moveLeft() {
  let moved = false;
  pushUndo();
  for (let i = 0; i < 4; i++) {
    const newRow = slide(matrix[i]);
    if (newRow.toString() !== matrix[i].toString()) moved = true;
    matrix[i] = newRow;
  }
  if (moved) spawnTile();
  renderGrid();
}

function moveRight() {
  pushUndo();
  matrix = matrix.map(row => row.reverse());
  moveLeft();
  matrix = matrix.map(row => row.reverse());
}

function moveUp() {
  pushUndo();
  matrix = rotateMatrix(matrix);
  moveLeft();
  matrix = rotateMatrix(rotateMatrix(rotateMatrix(matrix)));
}

function moveDown() {
  pushUndo();
  matrix = rotateMatrix(matrix);
  moveRight();
  matrix = rotateMatrix(rotateMatrix(rotateMatrix(matrix)));
}

function spawnTile() {
  let empty = [];
  matrix.forEach((row, i) =>
    row.forEach((cell, j) => {
      if (cell === 0) empty.push([i, j]);
    })
  );
  if (empty.length === 0) return;
  let [i, j] = empty[Math.floor(Math.random() * empty.length)];
  matrix[i][j] = Math.random() < 0.9 ? 2 : 4;
}

function undo() {
  if (undoStack.length === 0) return alert("Không thể undo!");
  redoStack.push({
    matrix: JSON.parse(JSON.stringify(matrix)),
    score: score
  });
  const prev = undoStack.pop();
  matrix = JSON.parse(JSON.stringify(prev.matrix));
  score = prev.score;
  renderGrid();
}

function redo() {
  if (redoStack.length === 0) return alert("Không thể redo!");
  undoStack.push({
    matrix: JSON.parse(JSON.stringify(matrix)),
    score: score
  });
  const next = redoStack.pop();
  matrix = JSON.parse(JSON.stringify(next.matrix));
  score = next.score;
  renderGrid();
}

function saveGame() {
  fetch('/api/save', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({
      username: username,
      matrix: matrix,
      score: score
    })
  })
  .then(res => res.json())
  .then(data => alert(data.reply || "Đã lưu thành công!"))
  .catch(err => alert("Lỗi khi lưu!"));
}

function loadGame() {
  fetch(`/api/load?username=${username}`)
    .then(res => res.json())
    .then(data => {
      if (data.matrix && data.score !== undefined) {
        matrix = data.matrix;
        score = data.score;
      } else {
        spawnTile(); spawnTile();
      }
      renderGrid();
    })
    .catch(err => {
      spawnTile(); spawnTile();
      renderGrid();
    });
}

function exitToMenu() {
  window.location.href = "/menu.html";
}

document.addEventListener("keydown", e => {
  switch (e.key) {
    case "ArrowLeft": moveLeft(); break;
    case "ArrowRight": moveRight(); break;
    case "ArrowUp": moveUp(); break;
    case "ArrowDown": moveDown(); break;
  }
});
