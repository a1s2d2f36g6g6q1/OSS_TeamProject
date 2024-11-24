const express = require('express');
const cors = require('cors');
const db = require('../db');

const app = express();

app.use(cors());

const router = express.Router();

router.use(cors()); // 라우터 수준에서 CORS 적용

// 로그인 경로
router.post('/login', (req, res) => {
    console.log('Received POST /login request');
    const { username, password } = req.body;

    if (!username || !password) {
        return res.status(400).json({ success: false, message: 'Username and password required' });
    }

    const query = 'SELECT * FROM users WHERE username = ?';
    db.query(query, [username], (err, results) => {
        if (err) {
            console.error('Database error:', err);
            return res.status(500).json({ success: false, message: 'Database error' });
        }

        if (results.length === 0 || results[0].password !== password) {
            return res.status(401).json({ success: false, message: 'Invalid username or password' });
        }

        res.json({ success: true, message: 'Login successful' });
    });
});

// 라우터 객체 내보내기
module.exports = router;
