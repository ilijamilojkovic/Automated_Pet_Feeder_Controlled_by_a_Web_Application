import { useState } from 'react';
import './Tajmer.css';
const Tajmer = ({ onTimeChange }) => {
  const [selectedTime, setSelectedTime] = useState("00:00");

  const handleTimeChange = (event) => {
    const time = event.target.value;
    setSelectedTime(time);
    onTimeChange(time);
  };

  return (
    <div>
        <h2>Odaberite vreme prvog obroka:</h2>
        <input
          id='timer'
          type="time"
          value={selectedTime}
          onChange={handleTimeChange}
        />
    </div>
  );
};

export default Tajmer;