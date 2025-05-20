document.getElementById("registerBtn").addEventListener("click", () => {
  const username = document.getElementById("username").value.trim();
  const password = document.getElementById("password").value.trim();
  const msg = document.getElementById("msg");

  // Kiểm tra nhập đầy đủ
  if (!username || !password) {
    msg.textContent = "Vui lòng nhập đủ tên đăng nhập và mật khẩu";
    return;
  }

  console.log("Đang gửi đăng ký với:", { username, password });

  fetch("/api/register", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ username, password }),
  })
    .then((res) => {
      console.log("Phản hồi từ server:", res.status);
      if (!res.ok) {
        return res.text().then((text) => {
          throw new Error(text);
        });
      }
      return res.text();
    })
    .then((data) => {
      console.log("Phản hồi thành công:", data);
      alert("Đăng ký thành công! Vui lòng đăng nhập.");
      window.location.href = "/login.html";
    })
    .catch((err) => {
      console.error("Lỗi khi đăng ký:", err);
      msg.textContent = err.message || "Lỗi khi đăng ký tài khoản";
    });
});
