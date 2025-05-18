function newGame() {
    fetch('/new-game', { method: 'POST' })
        .then(res => {
            if (res.ok) window.location.href = '/static/game.html';
            else alert("Không thể tạo game mới.");
        });
}

function resumeGame() {
    fetch('/resume')
        .then(res => {
            if (res.ok) window.location.href = '/static/game.html';
            else alert("Không có game để tiếp tục.");
        });
}

function exitGame() {
    fetch('/exit', { method: 'POST' })
        .then(res => res.text())
        .then(msg => {
            alert(msg);
            window.close(); // đóng tab
        });
}
