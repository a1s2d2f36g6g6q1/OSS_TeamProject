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

router.post('/save-score', (req, res) => {
    const { username, game, score } = req.body;

    const query = `
        INSERT INTO scores (user_id, username, game, high_score)
        VALUES (
            (SELECT id FROM users WHERE username = ?),
            ?, ?, ?
        )
        ON DUPLICATE KEY UPDATE high_score = GREATEST(high_score, ?);
    `;

    db.query(query, [username, username, game, score, score], (err) => {
        if (err) {
            console.error('Database error:', err);
            return res.status(500).json({ success: false, message: 'Database error' });
        }
        res.json({ success: true, message: 'Score updated successfully' });
    });
});

router.get('/get-high-score', (req, res) => {
    const { username, game } = req.query;

    if (!username || !game) {
        return res.status(400).json({ success: false, message: 'Username and game are required' });
    }

    const query = 'SELECT high_score FROM scores WHERE username = ? AND game = ?';

    db.query(query, [username, game], (err, results) => {
        if (err) {
            console.error('Database error:', err);
            return res.status(500).json({ success: false, message: 'Database error' });
        }

        const highScore = results.length > 0 ? results[0].high_score : 0;
        res.json({ success: true, highScore });
    });
});

// 라우터 객체 내보내기
module.exports = router;
