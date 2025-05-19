document.getElementById("loginBtn").addEventListener("click", () => {
  const username = document.getElementById("username").value.trim();
  const password = document.getElementById("password").value.trim();
  const msg = document.getElementById("msg");

  if (!username || !password) {
    msg.textContent = "Vui lòng nhập đủ tên đăng nhập và mật khẩu";
    return;
  }

  fetch("/api/login", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ username, password }),
  })
    .then((res) => {
      if (!res.ok) throw new Error("Đăng nhập thất bại");
      return res.text();
    })
    .then(() => {
      localStorage.setItem("username", username);
      window.location.href = "/game.html";
    })
    .catch((err) => {
      msg.textContent = "Tên đăng nhập hoặc mật khẩu không đúng";
    });
});
