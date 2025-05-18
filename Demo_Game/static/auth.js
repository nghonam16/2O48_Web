async function login() {
    const username = document.getElementById("username").value;
    const password = document.getElementById("password").value;

    const res = await fetch("/api/login", {
        method: "POST",
        body: JSON.stringify({ username, password })
    });

    const data = await res.json();
    document.getElementById("response").innerText = data.message;
}

async function register() {
    const username = document.getElementById("username").value;
    const password = document.getElementById("password").value;

    const res = await fetch("/api/register", {
        method: "POST",
        body: JSON.stringify({ username, password })
    });

    const data = await res.json();
    document.getElementById("response").innerText = data.message;
}
