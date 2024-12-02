const WebSocket = require('ws');

const rooms = {}; // 방 관리 객체

function setupWebSocketServer(server) {
    const wss = new WebSocket.Server({ server });

    wss.on('connection', (ws) => {
        console.log('New client connected.');
        ws.isAlive = true;

        ws.on('message', (data) => {
            try {
                const message = JSON.parse(data);
                console.log('Message received:', message);

                if (message.action === 'join') {
                    joinRoom(ws, message.roomId);
                } else if (message.action === 'message') {
                    // 방에 있는 다른 사용자에게 메시지 브로드캐스트
                    broadcastMessage(message.roomId, message.content, ws);
                } else {
                    ws.send(JSON.stringify({ action: 'error', message: 'Unknown action' }));
                }
            } catch (error) {
                console.error('Error processing message:', error);
                ws.send(JSON.stringify({ action: 'error', message: 'Invalid message format' }));
            }
        });

        ws.on('close', () => {
            leaveAllRooms(ws);
            console.log('Client disconnected.');
        });

        ws.on('pong', () => {
            ws.isAlive = true;
        });
    });

    setInterval(() => {
        wss.clients.forEach((socket) => {
            if (!socket.isAlive) {
                console.log('Terminating inactive socket.');
                return socket.terminate();
            }
            socket.isAlive = false;
            socket.ping();
        });
    }, 30000);

    function joinRoom(ws, roomId) {
        if (!rooms[roomId]) rooms[roomId] = [];
        rooms[roomId].push(ws);
        console.log(`Client joined room ${roomId}`);
        ws.send(JSON.stringify({ action: 'joined', roomId, message: `Joined room ${roomId}` }));
    }

    function broadcastMessage(roomId, content, senderSocket) {
        if (!rooms[roomId]) return;

        rooms[roomId].forEach((socket) => {
            if (socket !== senderSocket) {
                socket.send(JSON.stringify({ action: 'message', roomId, content }));
            }
        });
        console.log(`Broadcasted message to room ${roomId}: ${content}`);
    }

    function leaveAllRooms(ws) {
        for (const roomId in rooms) {
            rooms[roomId] = rooms[roomId].filter((socket) => socket !== ws);
            if (rooms[roomId].length === 0) delete rooms[roomId];
        }
    }
}
    

module.exports = setupWebSocketServer;
