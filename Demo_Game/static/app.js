async function register(username, password) {
  try {
    const response = await fetch('/api/register', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({ username, password })
    });
    const data = await response.json();
    if (response.ok) {
      alert('Đăng ký thành công: ' + data.reply);
    } else {
      alert('Lỗi đăng ký: ' + (data.error || 'Không xác định'));
    }
  } catch (error) {
    alert('Lỗi mạng hoặc server: ' + error.message);
  }
}

async function login(username, password) {
  try {
    const response = await fetch('/api/login', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({ username, password })
    });
    const data = await response.json();
    if (response.ok && !data.error) {
      alert('Đăng nhập thành công');
      // Lưu username lên localStorage hoặc sessionStorage để dùng tiếp
      sessionStorage.setItem('username', username);
      // Ví dụ gọi tiếp load trạng thái game
      await loadGame(username);
    } else {
      alert('Lỗi đăng nhập: ' + (data.error || 'Không xác định'));
    }
  } catch (error) {
    alert('Lỗi mạng hoặc server: ' + error.message);
  }
}

async function loadGame(username) {
  try {
    const response = await fetch(`/api/load?username=${encodeURIComponent(username)}`);
    const data = await response.json();
    if (response.ok) {
      if (data.matrix) {
        console.log('Load game thành công:', data);
        // Gán dữ liệu matrix, điểm vào game frontend của bạn
        // Ví dụ:
        // gameMatrix = data.matrix;
        // gameScore = data.score;
        // renderGame();
      } else {
        alert('Chưa có game lưu, bắt đầu game mới');
        // Gọi tạo game mới
        await newGame(username);
      }
    } else {
      alert('Lỗi khi tải game: ' + response.status);
    }
  } catch (error) {
    alert('Lỗi mạng hoặc server: ' + error.message);
  }
}

async function newGame(username) {
  try {
    const response = await fetch('/new-game', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({ username })
    });
    if (response.ok) {
      alert('Game mới đã được tạo!');
      // Load lại trạng thái mới (nếu cần)
      await loadGame(username);
    } else {
      alert('Lỗi tạo game mới');
    }
  } catch (error) {
    alert('Lỗi mạng hoặc server: ' + error.message);
  }
}

async function saveGame(username, gameMatrix, gameScore) {
  try {
    const response = await fetch('/api/save', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({
        username,
        matrix: gameMatrix,
        score: gameScore
      })
    });
    const data = await response.json();
    if (response.ok) {
      alert('Đã lưu game: ' + data.reply);
    } else {
      alert('Lỗi lưu game');
    }
  } catch (error) {
    alert('Lỗi mạng hoặc server: ' + error.message);
  }
}
