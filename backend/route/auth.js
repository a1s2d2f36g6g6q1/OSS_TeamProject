const express = require('express');
const cors = require('cors');
const db = require('../db');

const app = express();

app.use(cors());

const router = express.Router();

router.use(cors()); // 라우터 수준에서 CORS 적용

// 로그인 경로
router.post('/login', async (req, res) => {
    const { username, password } = req.body;

    if (!username || !password) {
        return res.status(400).json({ success: false, message: 'Username and password required' });
    }

    try {
        // MySQL 쿼리 실행
        const query = 'SELECT * FROM users WHERE username = ?';
        const [results] = await db.query(query, [username]);

        // 사용자 존재 여부 확인
        if (results.length === 0 || results[0].password !== password) {
            return res.status(401).json({ success: false, message: 'Invalid username or password' });
        }

        res.json({ success: true, message: 'Login successful' });
    } catch (err) {
        console.error('Database query error:', err);
        res.status(500).json({ success: false, message: 'Database error' });
    }
});

router.post('/save-score', async (req, res) => {
    const { username, game, score } = req.body;

    if (!username || !game || !score) {
        return res.status(400).json({ success: false, message: 'Invalid input' });
    }

    try {
        const query = game === 'mine'
            ? `
                INSERT INTO scores (user_id, username, game, high_score)
                VALUES (
                    (SELECT id FROM users WHERE username = ?),
                    ?, ?, ?
                )
                ON DUPLICATE KEY UPDATE high_score = LEAST(high_score, ?);
              `
            : `
                INSERT INTO scores (user_id, username, game, high_score)
                VALUES (
                    (SELECT id FROM users WHERE username = ?),
                    ?, ?, ?
                )
                ON DUPLICATE KEY UPDATE high_score = GREATEST(high_score, ?);
              `;

        const params = [username, username, game, score, score];

        const [result] = await db.execute(query, params);

        console.log('Score updated:', result);
        res.json({ success: true, message: 'Score updated successfully' });
    } catch (err) {
        console.error('Database error:', err);
        res.status(500).json({ success: false, message: 'Database error' });
    }
});



router.get('/get-all-scores', async (req, res) => {
    try {
        const query = `
            WITH RankedScores AS (
                SELECT 
                    game,
                    username,
                    high_score,
                    RANK() OVER (
                        PARTITION BY game 
                        ORDER BY 
                            CASE WHEN game = 'mine' THEN high_score
                                 ELSE -high_score
                            END ASC
                    ) AS \`rank\`
                FROM scores
            )
            SELECT 
                game, 
                username, 
                high_score, 
                \`rank\`
            FROM RankedScores
            WHERE \`rank\` <= 10
            ORDER BY game, \`rank\`;
        `;
        const [rows, fields] = await db.execute(query);

        console.log('Query result:', rows); // 결과 디버깅
        res.status(200).json({ gameScores: rows });
    } catch (err) {
        console.error('Database query error:', err);
        res.status(500).json({ error: 'Failed to fetch scores from database' });
    }
});

router.get('/get-user-scores', async (req, res) => {
    const { username } = req.query;

    if (!username) {
        return res.status(400).json({ success: false, message: 'Username required' });
    }

    try {
        const query = `
            SELECT 
                game, 
                high_score
            FROM scores
            WHERE username = ?
            ORDER BY game;
        `;

        const [rows] = await db.execute(query, [username]);

        res.status(200).json({ success: true, userScores: rows });
    } catch (err) {
        console.error('Database query error:', err);
        res.status(500).json({ success: false, message: 'Failed to fetch user scores' });
    }
});

router.get('/check-username', async (req, res) => {
    const { username } = req.query;

    if (!username) {
        return res.status(400).json({ success: false, message: 'Username is required' });
    }

    try {
        const query = 'SELECT COUNT(*) AS count FROM users WHERE username = ?';
        const [results] = await db.execute(query, [username]);

        if (results[0].count > 0) {
            return res.status(200).json({ success: false, message: 'Username already exists' });
        }

        res.status(200).json({ success: true, message: 'Username is available' });
    } catch (err) {
        console.error('Database query error:', err);
        res.status(500).json({ success: false, message: 'Database error' });
    }
});

router.post('/register', async (req, res) => {
    const { username, password } = req.body;

    if (!username || !password) {
        return res.status(400).json({ success: false, message: 'Username and password are required' });
    }

    try {
        const query = 'INSERT INTO users (username, password) VALUES (?, ?)';
        await db.execute(query, [username, password]);

        res.status(201).json({ success: true, message: 'User registered successfully' });
    } catch (err) {
        if (err.code === 'ER_DUP_ENTRY') {
            return res.status(409).json({ success: false, message: 'Username already exists' });
        }
        console.error('Database query error:', err);
        res.status(500).json({ success: false, message: 'Database error' });
    }
});



// 라우터 객체 내보내기
module.exports = router;
