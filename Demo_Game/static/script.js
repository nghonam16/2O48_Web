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

