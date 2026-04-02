// URL dell'ESP32 per i dati JSON ESEMPIO
const DATA_URL = "http://192.168.4.1/data"; // sostituire con l'IP reale!!!!!!!!!!!!!!!!!!!!!!1

async function updateData() {
  try {
    const response = await fetch(DATA_URL);
    const data = await response.json();
    const file = new File()

    // Aggiorna la velocità
    // document.getElementById("speed").innerText = data.speed.toFixed(2) + " m/s";


    // Aggiorna lo stato LED 
    const led = document.getElementById("ledStatus");

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

    //se arriva string
    var speed = parseFloat(data.speed); // Converti la stringa in numero
    var sensorTriggered = data.sensorTriggered === "true"; // Converti la stringa in booleano

    // Controlla se la velocità supera i 20 m/s (72 km/h)
    if (speed > 20) {
      // Se il sensore è stato attivato, mostra un avviso
      if (sensorTriggered) {
        console.log("Velocità eccessiva! Rilevato superamento del limite di ... m/s.");  //da cambiare con un messaggio più specifico
        document.getElementById("speed").innerText = speed + " m/s";
        led.classList.remove("bg-green-500");
        led.classList.add("bg-red-500");

        salva(data); // Salva i dati in un file JSON
      }else{
        console.log("ERRORE, MALCONFORMITà NEI DATI + " + data.speed + ", " + data.sensorTriggered);
        document.getElementById("speed").innerText = speed + " m/s";
        led.classList.remove("bg-green-500");
        led.classList.remove("bg-red-500");
        led.classList.add("bg-gray-500");

        salva(data); // Salva i dati in un file JSON  
      }
    }else{
        console.log("Velocità normale: " + speed + " m/s.");
        document.getElementById("speed").innerText = speed + " m/s";
        led.classList.remove("bg-red-500");
        led.classList.add("bg-green-500");

        salva(data); // Salva i dati in un file JSON
    }
    

  } catch (error) {
    console.error("Error fetching data:", error);
  }
}

// Aggiorna ogni secondo
setInterval(updateData, 1000);
// updateData(); // Chiamata iniziale //non seve a nulla in teoria da testare 


//==============================TEMP --> PASSARE A NODEJS===============================

//dato 1
    //crea file se non esiste 
    //metti dato  
    //elaborazione


/**
 * funzione per salvare i dati in un file JSON scaricabile 
 * @param {*} dati json da salvare in un file scaricabile
 */
function salva(dati) {
  const blob = new Blob(
    [JSON.stringify(dati, null, 2)],
    { type: "application/json" }
  );

  const a = document.createElement("a");
  a.href = URL.createObjectURL(blob);
  a.download = "dati.json";
  a.click();
}



//==============================TEMP NODEJS===============================
fetch("/salva", {
  method: "POST",
  headers: {
    "Content-Type": "application/json"
  },
  body: JSON.stringify(dati) // dati da salvare, da definire
});