const express = require('express');
const bodyParser = require('body-parser');
const cors = require('cors'); // CORS 모듈 가져오기
const authRoutes = require('./route/auth'); // auth.js 가져오기

const app = express();
const PORT = 5000;

app.use(bodyParser.json()); // JSON 요청 처리
app.use(cors()); // CORS 적용

app.use('/auth', authRoutes); // '/auth' 경로에 authRoutes 사용

app.listen(PORT, '0.0.0.0', () => {
    console.log(`Server running on http://0.0.0.0:${PORT}`);
});
    