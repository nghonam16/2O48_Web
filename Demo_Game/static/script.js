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
