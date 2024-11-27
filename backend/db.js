const mysql = require('mysql2');

// MySQL 데이터베이스 연결 설정
const db = mysql.createConnection({
    host: 'localhost', // 데이터베이스 호스트
    user: 'root',      // 데이터베이스 사용자
    password: '1248',  // 데이터베이스 비밀번호
    database: 'login_app', // 사용할 데이터베이스 이름
});

// 연결 확인
db.connect((err) => {
    if (err) {
        console.error('Error connecting to the database:', err);
        return;
    }
    console.log('Connected to MySQL database');
});

module.exports = db;
