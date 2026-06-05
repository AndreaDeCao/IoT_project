//file nodejs per salvataggio dati
// da rivedere

const express = require('express');
const fs = require('fs');
const path = require('path');

const app = express();

// const PORT = 3000;
const PORT = process.env.PORT || 3000;

let currentTestCaseId = 0; // Contatore per i test case
let timestamp = null; 

// Crea la cartella "data_sample" se non esiste
const dirPath = path.join(__dirname, 'data_sample');

if (!fs.existsSync(dirPath)) {
  fs.mkdirSync(dirPath);
}

// Percorso del file CSV
const filePath = path.join(dirPath, 'speed-log.csv'); // Percorso del file CSV

// per leggere JSON
app.use(express.json()); // Middleware per il parsing del JSON

// Crea il file CSV se non esiste e aggiungi l'intestazione
if (!fs.existsSync(filePath)) {
  fs.writeFileSync(filePath, 'Test-Case-ID,Speed,Sensor-Triggered,Timestamp\n');
}

const data = fs.readFileSync(filePath, 'utf-8');
const lines = data.trim().split('\n');

if (lines.length > 1) {
  const lastLine = lines[lines.length - 1];
  const lastId = parseInt(lastLine.split(',')[0]);

  if (!isNaN(lastId)) {
    currentTestCaseId = lastId;
  }

}

// serve frontend
app.use(express.static('public')); // Serve i file statici dalla cartella "public"

// // endpoint per salvare dati
// app.post('/salva', (req, res) => {
//   const dati = req.body;

//   fs.appendFile(
//     filePath, JSON.stringify(dati) + '\n', (err) => {
//       if (err) {
//         console.error(err);
//         return res.status(500).send("Errore");
//       }
//       res.send("Dati salvati");
//     }
//   );
// });

/**
 * Endpoint per salvare i dati nel file CSV
 */
app.post('/salva', (req, res) => {
  // const { speed, sensorTriggered, timestamp } = req.body;
  const { speed, sensorTriggered} = req.body;

  // Leggi l'ultima riga del file CSV
  let lastRow = null;
  if (fs.existsSync(filePath)) {
    const data = fs.readFileSync(filePath, 'utf-8').trim();
    const lines = data.split('\n');
    if (lines.length > 1) {
      lastRow = lines[lines.length - 1].split(',');
    }
  }
  
  timestamp = new Date().toISOString(); // Aggiorna il timestamp al momento della ricezione dei dati

  // Controllo duplicato: se ultima riga è identica a questa, non salvare
  if (lastRow &&
      lastRow[1] === String(speed) &&
      lastRow[2] === String(sensorTriggered) &&
      lastRow[3] === String(timestamp)) {
    return res.status(200).send({ message: "Riga già presente, non salvata" });
  }

  // Se nuova riga, incrementa ID e salva
  currentTestCaseId++; // Incrementa l'ID del test case

  const row = `${currentTestCaseId},${speed},${sensorTriggered},${timestamp}\n`;

  fs.appendFile(filePath, row, (err) => {
    if (err) {
      console.error(err);
      return res.status(500).send("Errore");
    }
    res.send({
      message: "Dati salvati",
      //si può togliere l'ID se non serve, ma potrebbe essere utile per identificare i test case in modo univoco
      id: currentTestCaseId // Restituisci l'ID del test case appena salvato
    });
  });
});


/**
 * Questo endpoint legge i dati dal file CSV, li elabora e restituisce un array di oggetti JSON.
 * Ogni oggetto rappresenta una riga del file CSV, con le proprietà "id", "speed", "sensorTriggered" e "timestamp".
 * Il file CSV deve avere un'intestazione con i nomi delle colonne corrispondenti.
 * Serve per fornire i dati al frontend in formato JSON, che poi possono essere visualizzati in una tabella o utilizzati per altre funzionalità.
 */
app.get('/dati', (req, res) => {
  fs.readFile(filePath, 'utf-8', (err, data) => {
    if (err) {
      console.error(err);
      return res.status(500).send("Errore lettura file");
    }

    const lines = data.trim().split('\n');
    const headers = lines[0].split(',');

    const result = lines.slice(1).map(line => {
      const values = line.split(',');
      return {
        id: values[0],
        speed: values[1],
        sensorTriggered: values[2],
        timestamp: values[3]
      };
    });

    res.json(result);
  });
});

/**
 * Legge l'ultima riga del file CSV e restituisce un oggetto JSON con le proprietà "speed" e "sensorTriggered".
 * Se il file CSV è vuoto o contiene solo l'intestazione, restituisce valori di default (speed: "0", sensorTriggered: "false").
 * Serve per aggiornare in tempo reale i dati visualizzati nel frontend, ad esempio per mostrare la velocità attuale e lo stato del sensore.
 */
app.get('/data', (req, res) => {
  fs.readFile(filePath, 'utf-8', (err, data) => {
    if (err) {
      return res.status(500).send("Errore");
    }

    const lines = data.trim().split('\n');

    if (lines.length <= 1) {
      return res.json({
        speed: "0",
        sensorTriggered: "false"
      });
    }

    const lastLine = lines[lines.length - 1];
    const values = lastLine.split(',');

    res.json({
      speed: values[1],
      sensorTriggered: values[2]
    });
  });
});

app.use('/firmware', express.static(path.join(__dirname, '../../firmware')));

app.listen(PORT, () => {
  console.log(`Server attivo su http://localhost:${PORT}`);
});



// per avvio 
// npm init -y
// npm install express
// node server.js