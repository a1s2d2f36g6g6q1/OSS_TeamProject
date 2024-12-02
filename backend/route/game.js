const express = require('express');
const router = express.Router();
const db = require('../db'); // 데이터베이스 연결

// 방 생성
router.post('/create', (req, res) => {
    const { name } = req.body;

    if (!name) {
        return res.status(400).json({ success: false, message: "Room name is required." });
    }

    const query = 'INSERT INTO rooms (name) VALUES (?)';
    db.query(query, [name], (err, results) => {
        if (err) {
            console.error('Database error during room creation:', err);
            return res.status(500).json({ success: false, message: 'Database error', error: err.sqlMessage });
        }
        const roomId = results.insertId;
        res.json({ success: true, roomId, message: `Room '${name}' created successfully.` });
    });
});

// 방 참가
router.post('/join', (req, res) => {
    const { roomId } = req.body;
    const playerIp = req.ip;

    const checkRoomQuery = 'SELECT * FROM rooms WHERE id = ?';
    db.query(checkRoomQuery, [roomId], (err, results) => {
        if (err) {
            console.error('Database error:', err);
            return res.status(500).json({ success: false, message: 'Database error' });
        }
        if (results.length === 0) {
            return res.status(404).json({ success: false, message: 'Room not found' });
        }

        const joinRoomQuery = 'INSERT INTO room_players (room_id, player_ip) VALUES (?, ?)';
        db.query(joinRoomQuery, [roomId, playerIp], (err) => {
            if (err) {
                console.error('Database error:', err);
                return res.status(500).json({ success: false, message: 'Database error' });
            }
            res.json({ success: true, message: `Joined room ${roomId} successfully.` });
        });
    });
});

// 메시지 전송
router.post('/message', (req, res) => {
    const { roomId, message } = req.body;

    const checkRoomQuery = 'SELECT * FROM rooms WHERE id = ?';
    db.query(checkRoomQuery, [roomId], (err, results) => {
        if (err) {
            console.error('Database error:', err);
            return res.status(500).json({ success: false, message: 'Database error' });
        }
        if (results.length === 0) {
            return res.status(404).json({ success: false, message: 'Room not found' });
        }

        console.log(`Message from Room ${roomId}: ${message}`);
        res.json({ success: true, message: 'Message sent successfully.' });
    });
});

module.exports = router;
