// URL dell'ESP32 per i dati JSON ESEMPIO
const DATA_URL = "http://192.168.4.1/data"; // sostituire con l'IP reale!!!!!!!!!!!!!!!!!!!!!!1

async function updateData() {
  try {
    const response = await fetch(DATA_URL);
    const data = await response.json();

    //dato 1
    //crea file se non esiste 
    //metti dato  
    //elaborazione

    // Aggiorna la velocità
    document.getElementById("speed").innerText = data.speed.toFixed(2) + " m/s";


    // Aggiorna lo stato LED
    const led = document.getElementById("ledStatus");
    if (data.led) {
      led.classList.remove("bg-red-500");
      led.classList.add("bg-green-500");
    } else {
      led.classList.remove("bg-green-500");
      led.classList.add("bg-red-500");
    }
  } catch (error) {
    console.error("Error fetching data:", error);
  }
}

// Aggiorna ogni secondo
setInterval(updateData, 1000);
// updateData(); // Chiamata iniziale //non seve a nulla in teoria da testare 