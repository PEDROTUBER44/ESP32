function login() {
    const username = document.getElementById('username').value;
    const password = document.getElementById('password').value;

    if (username == 'user123' && password == '214365') {
        window.location.href = "access.html";
    } else {
        alert('Usuário ou senha incorretos.');
    }
}

function openCamera() {
    const video = document.getElementById('cameraFeed');
    if (navigator.mediaDevices && navigator.mediaDevices.getUserMedia) {
        navigator.mediaDevices.getUserMedia({ video: true })
            .then(function (stream) {
                video.srcObject = stream;
            })
            .catch(function (error) {
                alert('Não foi possível acessar a câmera: ' + error.message);
            });
    } else {
        alert('Seu navegador não suporta acesso à câmera.');
    }
}

function unlockDoor() {
    // Enviar um comando para o ESP para desbloquear a fechadura.
    fetch('/unlock')
        .then(response => response.text())
        .then(data => alert('Fechadura desbloqueada'))
        .catch(error => alert('Erro ao desbloquear a fechadura: ' + error.message));
}