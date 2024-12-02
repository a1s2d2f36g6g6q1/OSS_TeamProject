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
                    // 필드 유효성 검사 추가
                    if (typeof message.roomId === 'undefined' || typeof message.content === 'undefined') {
                        console.error('Invalid message format: Missing roomId or content.');
                        ws.send(JSON.stringify({ action: 'error', message: 'Missing roomId or content field.' }));
                        return;
                    }
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
        if (!rooms[roomId]) {
            console.error(`Room ${roomId} does not exist or is empty.`);
            return;
        }
    
        rooms[roomId].forEach((socket) => {
            if (socket !== senderSocket) {
                // JSON 형식으로 보내기 전에 검사
                const messageToSend = JSON.stringify({ action: 'message', roomId: roomId, content: content });
                console.log(`Broadcasting message to room ${roomId}: ${messageToSend}`); // 로그 추가
                socket.send(messageToSend);
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
