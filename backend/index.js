const express = require('express');
const bodyParser = require('body-parser');
const cors = require('cors');
const http = require('http');

const authRoutes = require('./route/auth'); // auth.js 가져오기

const app = express();

const appPORT = 5000;

// Express 미들웨어 설정
app.use(bodyParser.json());
app.use(cors());
app.use((req, res, next) => {
    res.setHeader('Connection', 'keep-alive');
    next();
});
app.use('/auth', authRoutes); // '/auth' 경로에 authRoutes 사용

app.listen(appPORT,'0.0.0.0', () => {
    console.log(`app running on http://0.0.0.0:${appPORT}`);
});

