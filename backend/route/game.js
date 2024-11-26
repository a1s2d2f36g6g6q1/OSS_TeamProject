const express = require('express');
const router = express.Router();
const db = require('../db'); // DB 연결

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