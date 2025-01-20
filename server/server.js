const http = require('http');
const fs = require('fs');
const WebSocket = require('ws');

const PORT = 3000;
let receivedData = ''; // for json

const server = http.createServer((req, res) => {
    if (req.method === 'POST' && req.headers['content-type'] === 'application/json') {
        // POST 
        let body = '';
        req.on('data', chunk => {
            body += chunk.toString();
        });

        req.on('end', () => {
            receivedData = body; // paring json data
            console.log('Received Data:', receivedData);

            // WebSocket send
            wss.clients.forEach(client => {
                if (client.readyState === WebSocket.OPEN) {
                    client.send(receivedData);
                }
            });

            res.writeHead(200, { 'Content-Type': 'application/json' });
            res.end(JSON.stringify({ message: 'Data received successfully' }));
        });
    } else if (req.method === 'GET') {
        // GET 
        fs.readFile('page.html', 'utf8', (err, htmlTemplate) => {
            if (err) {
                res.writeHead(500, { 'Content-Type': 'text/plain' });
                res.end('Internal Server Error');
                return;
            }
            res.writeHead(200, { 'Content-Type': 'text/html' });
            res.end(htmlTemplate);
        });
    } else {
        res.writeHead(404, { 'Content-Type': 'text/plain' });
        res.end('Not Found');
    }
});

// WebSocket server 
const wss = new WebSocket.Server({ server });

server.listen(PORT, () => {
    console.log(`Server running at http://localhost:${PORT}`);
});
