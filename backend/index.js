const express = require('express');
const bodyParser = require('body-parser');
const cors = require('cors');
const http = require('http');
const setupWebSocketServer = require('./websocket'); // WebSocket 서버 가져오기

const authRoutes = require('./route/auth'); // auth.js 가져오기
const gameRoutes = require('./route/game');

const app = express();
const server = http.createServer(app); // HTTP 서버 생성

const appPORT = 5000;
const serverPORT = 6000;

// Express 미들웨어 설정
app.use(bodyParser.json());
app.use(cors());
app.use((req, res, next) => {
    res.setHeader('Connection', 'keep-alive');
    next();
});
app.use('/auth', authRoutes); // '/auth' 경로에 authRoutes 사용
app.use('/game', gameRoutes);

server.keepAliveTimeout = 60000; // 1 minute
server.headersTimeout = 65000; // Slightly more than Keep-Alive timeout
// WebSocket 서버 설정
setupWebSocketServer(server);


// HTTP 서버 실행
server.listen(serverPORT,'0.0.0.0', () => {
    console.log(`Server running on http://0.0.0.0:${serverPORT}`);
});
app.listen(appPORT,'0.0.0.0', () => {
    console.log(`app running on http://0.0.0.0:${appPORT}`);
});

