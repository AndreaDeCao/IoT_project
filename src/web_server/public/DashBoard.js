// URL dell'ESP32 per i dati JSON ESEMPIO
// const DATA_URL = "http://192.168.4.1/data"; // sostituire con l'IP reale!!!!!!!!!!!!!!!!!!!!!!1

// const { existsSync } = require("node:fs");

// const DATA_URL = "http://localhost:3000/"; // per test locale
const DATA_URL = "http://localhost:3000/data";

let lastSpeed = null; // Variabile per memorizzare l'ultima velocità
let lastSensorTriggered = null; // Variabile per memorizzare l'ultimo stato del sensore

// async function updateDashBoard() {
//   const response = await fetch("/data"); // per test locale, sostituire con DATA_URL per l'ESP32
//   const data = await response.json();

// }

async function updateData() {
  try {
    // const response = await fetch(DATA_URL);
    const response = await fetch("/data"); // per test locale, sostituire con DATA_URL per l'ESP32
    const data = await response.json();

    // Aggiorna lo stato LED con valore ON/OFF e cambia colore
    // if (data.led == "OFF") {    
    //   led.classList.remove("bg-red-500");
    //   led.classList.add("bg-green-500");
    // } else {
    //   led.classList.remove("bg-green-500");
    //   led.classList.add("bg-red-500");
    // }

    //==============================ELABORAZIONE DATI===============================

    // var speed = data.speed; // Supponendo che il sensore invii un campo "speed" in m/s
    // var sensorTriggered = data.sensorTriggered; // Supponendo che il sensore invii un campo booleano "sensorTriggered"

    var speed = parseFloat(data.speed); // Converti la stringa in numero
    var sensorTriggered = data.sensorTriggered === "true"; // Converti la stringa in booleano

    // Aggiorna la dashboard solo se cambia qualcosa
    if (speed !== lastSpeed || sensorTriggered !== lastSensorTriggered) {

      // Aggiorna lo stato LED 
      const led = document.getElementById("ledStatus");

      var limit = 0.6;  //TODO: cambia per valore multa




      // Controlla se la velocità supera i 3 m/s (10.8 km/h)
      if (speed > limit) {
        // Se il sensore è stato attivato, mostra un avviso
        if (sensorTriggered) {
          console.log("Velocità eccessiva! Rilevato superamento del limite di " + (speed - limit) + " m/s. ");  //da cambiare con un messaggio più specifico
          led.classList.remove("bg-green-500");
          led.classList.add("bg-red-500");
        }else{
          console.log("ERRORE, MALCONFORMITA' NEI DATI --> " + data.speed + " m/s, " + data.sensorTriggered);
          led.classList.remove("bg-green-500", "bg-red-500");
          led.classList.add("bg-gray-500");
        }
      }else{
        if(!sensorTriggered){
          console.log("Velocità normale: " + speed + " m/s, sensore non attivato.");
          led.classList.remove("bg-green-500", "bg-red-500");
          led.classList.add("bg-green-500");
        }else{
          console.log("ERRORE, MALCONFORMITA' NEI DATI --> " + data.speed + " m/s, " + data.sensorTriggered);
          led.classList.remove("bg-green-500", "bg-red-500");
          led.classList.add("bg-gray-500");
        }
      }

      document.getElementById("speed").innerText = speed + " m/s";

      // Aggiorna le variabili per la prossima verifica
      lastSpeed = speed;
      lastSensorTriggered = sensorTriggered;
    } 

    // i dati vengono salvati solo se la velocità è presente e il sensore è attivo, per evitare di salvare dati non significativi o malformati
    if(speed && sensorTriggered == null){
      await fetch("/salva", {
        method: "POST",
        headers: {
          "Content-Type": "application/json"
        },
        body: JSON.stringify({
          speed,
          sensorTriggered,
          timestamp: new Date().toISOString()
        })
      });
    }    

    document.getElementById("speed").innerText = speed + " m/s";

    speed = sensorTriggered = null; // Resetta le variabili dopo l'elaborazione

    // await loadLog();

  } catch (error) {
    console.error("Error fetching data:", error);
  }
}

async function loadLog() {
  try {
    const res = await fetch("/dati");
    const data = await res.json();

    const table = document.getElementById("logTable");
    table.innerHTML = "";

    // Inverti l'ordine dei dati per mostrare prima i più recenti
    data.reverse();

    data.forEach(entry => {
      const row = document.createElement("tr");

      row.innerHTML = `
        <td class="p-2">${entry.id}</td>
        <td class="p-2">${entry.speed}</td>
        <td class="p-2">${entry.sensorTriggered}</td>
        <td class="p-2 text-xs">${new Date(entry.timestamp).toLocaleTimeString()}</td>
      `;

      table.appendChild(row);
    });

  } catch (err) {
    console.error("Errore caricamento log:", err);
  }
}

// Aggiorna ogni secondo
setInterval(updateData, 1000);

// Carica il log ogni 2 secondi
setInterval(loadLog, 2000);