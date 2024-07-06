const express = require('express');
const bodyParser = require('body-parser');
const cors = require('cors');
const favicon = require('serve-favicon');
const path = require('path');
const app = express();
const axios = require('axios');

const arduinoUrl = 'http://192.168.36.219';
const PORT = 8000; 

app.listen(PORT, () => {
  const serverAddress = `http://localhost:${PORT}`;
  console.log(`Server started at ${serverAddress}`);
});
app.use(cors());
app.use(bodyParser.json());
app.use(favicon(path.join(__dirname, 'favicon.ico')));
app.get('/', (req, res) => {
  res.send('Dobrodošli na server!');
});


let sacuvanaKolicina;
let sacuvanaVrednostDugmeta;
let sacuvanoVreme;
let intervalId;

app.post('/api/saveData', (req, res) => {
  const { kolicina, vrednostDugmeta, selectedTime } = req.body;
  sacuvanaKolicina = kolicina;
  sacuvanaVrednostDugmeta = vrednostDugmeta;
  sacuvanoVreme = selectedTime;

  // Postavljamo interval koji će svakih 1 sekundu proveravati da li se vreme poklapa
  intervalId = setInterval(() => {
    let trenutnoVreme = new Date();
    const [selectedHours, selectedMinutes] = sacuvanoVreme.split(':').map(Number);

    // Proveri da li je trenutno vreme jednako vremenu iz selectedTime
    if (trenutnoVreme.getHours() === selectedHours && trenutnoVreme.getMinutes() === selectedMinutes) {
      clearInterval(intervalId); // Prekini interval kada se vreme poklopi
      console.log(`${sacuvanaKolicina},${sacuvanaVrednostDugmeta}`);
      sendToArduino(sacuvanaKolicina, sacuvanaVrednostDugmeta);
      console.log('Podaci za slanje na Arduino:', kolicina, vrednostDugmeta);
      res.json({ success: true, message: 'Podaci uspesno sacuvani!' });
    } else {
      console.log('Trenutno vreme se ne poklapa sa zadatim vremenom.');
    }
  }, 1000); // Interval svake sekunde
});

async function sendToArduino(kolicina, vrednostDugmeta) {
  try {
    const dataToSend = {
      k: kolicina,
      v: vrednostDugmeta,
    };

    const jsonString = JSON.stringify(dataToSend);
    console.log('Šaljem JSON podatke:', jsonString);

    const response = await axios.post(`${arduinoUrl}/80`, jsonString, {
      headers: {
        'Content-Type': 'application/json'
      }
    });

    console.log('Odgovor od Arduino-a:', response.data);
  } catch (error) {
    console.error('Greška prilikom slanja podataka na Arduino:', error.message);
  }
}

app.post('/notification', (req, res) => {
  const { message } = req.body;
  console.log('Notification received:', message);
  // Ovdje možete dodati kod za slanje notifikacije korisnicima
  res.status(200).send('Notification received');
});