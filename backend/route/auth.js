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

router.get('/get-all-scores', async (req, res) => {
    console.log('GET /auth/get-all-scores called'); // 요청 디버깅
    try {
        const query = `
            WITH RankedScores AS (
                SELECT 
                    game,
                    username,
                    high_score,
                    RANK() OVER (PARTITION BY game ORDER BY high_score DESC) AS \`rank\`
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
        console.log('Executing query:', query); // 쿼리 디버깅

        // 쿼리 실행
        const [rows, fields] = await db.execute(query);

        console.log('Query result:', rows); // 결과 디버깅
        res.status(200).json({ gameScores: rows });
    } catch (err) {
        console.error('Database query error:', err); // 에러 디버깅
        res.status(500).json({ error: 'Failed to fetch scores from database' });
    }
});


// 라우터 객체 내보내기
module.exports = router;
