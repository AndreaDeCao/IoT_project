/**
 * Simple interaction example
 */
document.addEventListener("DOMContentLoaded", () => {
  console.log("IoT Autovelox Page Loaded Successfully");
});

/*
document.getElementById("temp").addEventListener("click", () => {
  alert("You are downloading the temporary project report.");
});
*/

/**
 * It was used at the beginning to verify that the page is functioning correctly
 */
/*function showWelcomeMessage() {
    alert("Welcome to the IoT Project Documentation!");
}
showWelcomeMessage();*/

document.addEventListener("DOMContentLoaded", () => {
  const slider = document.getElementById("slider");
  const slides = slider.children;
  const total = slides.length;
  document.getElementById("prev").style.display = "none";
  document.getElementById("next").style.display = "none";

  let index = 0;

  // Imposta la larghezza totale del contenitore (importantissimo!)
  slider.style.width = `${total * 100}%`;

  // Ogni immagine deve occupare la stessa parte dello slider
  for (let s of slides) {
    s.style.width = `${100 / total}%`;
  }
  document.getElementById("slider-container").onmouseenter = () => {
    document.getElementById("prev").style.display="block";
    document.getElementById("next").style.display = "block";
  }
  document.getElementById("slider-container").onmouseleave = () => {
    document.getElementById("prev").style.display="none";
    document.getElementById("next").style.display = "none";
  }

  function updateSlider() {
    slider.style.transform = `translateX(-${index * (100 / total)}%)`;
  }

  document.getElementById("next").onclick = () => {
    index = (index + 1) % total;
    updateSlider();
  };

  document.getElementById("prev").onclick = () => {
    index = (index - 1 + total) % total;
    updateSlider();
  };
});


