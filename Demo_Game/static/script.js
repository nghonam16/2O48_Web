document.addEventListener("DOMContentLoaded", () => {
    const messageP = document.getElementById("message");

    fetch("/api/hello", {
        method: "POST",
        headers: {
            "Content-Type": "application/json"
        },
        body: JSON.stringify({ name: "2048 Player" })
    })
    .then(res => res.json())
    .then(data => {
        messageP.textContent = data.reply;
    })
    .catch(err => {
        messageP.textContent = "Lỗi khi kết nối backend.";
        console.error(err);
    });
});

function saveGameState(username, matrix, score) {
    fetch('/api/save', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({
            username: username,
            matrix: matrix,
            score: score
        })
    })
    .then(res => res.json())
    .then(data => {
        alert(data.reply);
    })
    .catch(err => console.error('Lỗi:', err));
}

function loadGameState(username) {
    fetch('/api/load', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ username: username })
    })
    .then(res => res.json())
    .then(data => {
        if (data.error) {
            alert(data.error);
            return;
        }

        const score = data.score;
        const matrix = data.matrix;

        // Giả sử bạn có hàm để in game ra HTML
        console.log("Tải game:", matrix, "Điểm:", score);
        alert(`Game đã được tải cho ${username}\nĐiểm: ${score}`);

        // TODO: cập nhật UI tương ứng với ma trận và điểm
    })
    .catch(err => {
        console.error('Lỗi khi tải game:', err);
        alert("Không thể tải trạng thái game.");
    });
}

let undoStack = [];
let redoStack = [];

function pushUndo() {
  undoStack.push({
    matrix: JSON.parse(JSON.stringify(matrix)),
    score: score
  });
  // Khi có hành động mới, xóa redoStack
  redoStack = [];
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

