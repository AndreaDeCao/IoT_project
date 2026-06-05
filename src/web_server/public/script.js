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

  const slider2 = document.getElementById("slider-2");
  const slides2 = slider2.children;
  const total2 = slides2.length;
  document.getElementById("prev-2").style.display = "none";
  document.getElementById("next-2").style.display = "none";

  let index = 0;
  let index2 = 0;


  // Imposta la larghezza totale del contenitore (importantissimo!)
  slider.style.width = `${total * 100}%`;
  slider2.style.width = `${total2 * 100}%`;

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

  for (let k of slides2) {
    k.style.width = `${100 / total2}%`;
  }
  document.getElementById("slider-container-2").onmouseenter = () => {
    document.getElementById("prev-2").style.display="block";
    document.getElementById("next-2").style.display = "block";
  }
  document.getElementById("slider-container-2").onmouseleave = () => {
    document.getElementById("prev-2").style.display="none";
    document.getElementById("next-2").style.display = "none";
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

  function updateSlider2() {
    slider2.style.transform = `translateX(-${index2 * (100 / total2)}%)`;
  }

  document.getElementById("next-2").onclick = () => {
    index2 = (index2 + 1) % total2;
    updateSlider2();
  };

  document.getElementById("prev-2").onclick = () => {
    index2 = (index2 - 1 + total2) % total2;
    updateSlider2();
  };
});


