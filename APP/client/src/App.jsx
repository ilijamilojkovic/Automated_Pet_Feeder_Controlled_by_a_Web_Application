import { useState } from 'react'
import Kolicina from './components/Kolicina'
import Dugmici from './components/Dugmici'
import Tajmer from './components/Tajmer'
import PotvrdniPopUp from './components/PotvrdniPopUp'
import './App.css'

function App() {
  const [kolicina, setKolicina] = useState(0);
  const [vrednostDugmeta, setVrednostDugmeta] = useState(null);
  const [selectedTime, setSelectedTime] = useState('00:00');
  const [isModalOpen, setIsModalOpen] = useState(false);

  const handleAmountChange = (amount) => {
    setKolicina(amount);
  }
  const handleTimeChange = (time) => {
    setSelectedTime(time);
  };

  const handleButtonClick = (value) => {
    setVrednostDugmeta(value);
  };

  const handleConfirmButtonClick = () => {
    setIsModalOpen(true);
  };

  const handleConfirmModalClose = () => {
    setIsModalOpen(false);
  };

  const handleConfirmModalConfirm = () => {
    posaljiNaBackend();
    setIsModalOpen(false);
  };

  const posaljiNaBackend = async () => {
    try {
      const response = await fetch('http://localhost:8000/api/saveData', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          kolicina,
          vrednostDugmeta,
          selectedTime,
        }),
      });
  
      if (!response.ok) {
        throw new Error('Neuspešan zahtev na backend.');
      }
  
      // Ovde možete obraditi odgovor od servera ako je potrebno
      const data = await response.json();
      console.log('Odgovor sa servera:', data);
    } catch (error) {
      console.error('Greška prilikom slanja podataka na back:', error.message);
    }
  };
  

  return (
    <>

      <h1>Pametna hranilica</h1>
      <div id='container'>
        <Kolicina onAmountChange={handleAmountChange} />
        <Dugmici onButtonClick={handleButtonClick} />
        <p style={{ color: vrednostDugmeta !== null ? 'white' : 'red'}} >
          {vrednostDugmeta !== null ? `Odabrali ste: ${vrednostDugmeta}h` : 'Niste odabrali razmak izmedju obroka!!!'}
        </p>
        <Tajmer onTimeChange={handleTimeChange} />
        <p>{selectedTime !== null ? `Odabrano vreme prvog obroka je: ${selectedTime}h` : 'Vreme prvog obroka nije odabrano!!!'}</p>
        <button onClick={handleConfirmButtonClick}>Potvrdi</button>
        
        <PotvrdniPopUp
          isOpen={isModalOpen}
          onRequestClose={handleConfirmModalClose}
          onConfirm={handleConfirmModalConfirm}
          kolicina={kolicina}
          ponavljanje={vrednostDugmeta}
          vreme={selectedTime}
        />
      </div>
    </>
  )
}

export default App
