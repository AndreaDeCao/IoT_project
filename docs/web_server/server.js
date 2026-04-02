//file nodejs per salvataggio dati
// da rivedere

const express = require('express');
const fs = require('fs');
const path = require('path');

const app = express();
const PORT = 3000;

const filePath = path.join(__dirname, '../data_sample/speed-log.csv'); // Percorso del file CSV

// per leggere JSON
app.use(express.json());

// serve frontend
app.use(express.static('public'));

// endpoint per salvare dati
app.post('/salva', (req, res) => {
  const dati = req.body;

  fs.appendFile(
    filePath, JSON.stringify(dati) + '\n', (err) => {
      if (err) {
        console.error(err);
        return res.status(500).send("Errore");
      }
      res.send("Dati salvati");
    }
  );
});

app.listen(PORT, () => {
  console.log(`Server attivo su http://localhost:${PORT}`);
});



// per avvio 
// npm init -y
// npm install express
// node server.js