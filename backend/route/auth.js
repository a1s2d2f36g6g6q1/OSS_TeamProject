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
                            CASE 
                                WHEN game = 'mine' THEN high_score
                                ELSE -high_score
                            END ASC
                    ) AS \`rank\`
                FROM scores
                WHERE high_score > 0
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

        const [rows] = await db.execute(query);

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
            WHERE username = ? AND high_score > 0
            ORDER BY game;
        `;

        const [rows] = await db.execute(query, [username]);

        res.status(200).json({ success: true, userScores: rows });
    } catch (err) {
        console.error('Database query error:', err);
        res.status(500).json({ success: false, message: 'Failed to fetch user scores' });
    }
});


// 닉네임 중복 확인 엔드포인트
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

// 계정 생성 엔드포인트
router.post('/register', async (req, res) => {
    const { username, password } = req.body;

    if (!username || !password) {
        return res.status(400).json({ success: false, message: 'Username and password are required' });
    }

    const connection = await db.getConnection(); // 트랜잭션 시작을 위해 커넥션 가져오기
    try {
        await connection.beginTransaction(); // 트랜잭션 시작

        const insertUserQuery = 'INSERT INTO users (username, password) VALUES (?, ?)';
        const [userResult] = await connection.execute(insertUserQuery, [username, password]);

        const userId = userResult.insertId; // 새로 생성된 사용자 ID 가져오기

        const insertScoresQuery = `
            INSERT INTO scores (user_id, username, game, high_score)
            VALUES (?, ?, '2048', 0), (?, ?, 'mine', 0), (?, ?, 'breakout', 0);
        `;
        await connection.execute(insertScoresQuery, [userId, username, userId, username, userId, username]);

        await connection.commit(); // 트랜잭션 커밋

        res.status(201).json({ success: true, message: 'User registered successfully' });
    } catch (err) {
        await connection.rollback(); // 오류 발생 시 트랜잭션 롤백

        if (err.code === 'ER_DUP_ENTRY') {
            return res.status(409).json({ success: false, message: 'Username already exists' });
        }
        console.error('Database query error:', err);
        res.status(500).json({ success: false, message: 'Database error' });
    } finally {
        connection.release(); // 연결 반환
    }
});


// Delete Account Endpoint
router.delete('/delete-account', async (req, res) => {
const username = req.query.username;

if (!username) {
    return res.status(400).json({ success: false, message: 'Username is required' });
}

try {
    // Delete user and related scores
    const deleteScoresQuery = 'DELETE FROM scores WHERE username = ?';
    const deleteUserQuery = 'DELETE FROM users WHERE username = ?';

    await db.execute(deleteScoresQuery, [username]);
    await db.execute(deleteUserQuery, [username]);

    res.status(200).json({ success: true, message: 'Account deleted successfully' });
} catch (err) {
    console.error('Database query error:', err);
    res.status(500).json({ success: false, message: 'Failed to delete account' });
}
});


module.exports = router;
