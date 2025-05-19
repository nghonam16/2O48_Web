document.getElementById("registerBtn").addEventListener("click", () => {
  const username = document.getElementById("username").value.trim();
  const password = document.getElementById("password").value.trim();
  const msg = document.getElementById("msg");

  if (!username || !password) {
    msg.textContent = "Vui lòng nhập đủ tên đăng nhập và mật khẩu";
    return;
  }

  fetch("/api/register", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ username, password }),
  })
    .then((res) => {
      if (!res.ok) return res.text().then(text => { throw new Error(text); });
      return res.text();
    })
    .then(() => {
      alert("Đăng ký thành công! Vui lòng đăng nhập.");
      window.location.href = "/login.html";
    })
    .catch((err) => {
      msg.textContent = err.message || "Lỗi khi đăng ký tài khoản";
    });
});
