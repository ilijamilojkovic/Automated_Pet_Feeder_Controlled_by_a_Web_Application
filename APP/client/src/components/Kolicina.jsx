import { useState } from 'react';

import './Kolicina.css';
const Kolicina = ({ onAmountChange }) => {

    const [selectedAmount, setSelectedAmont] = useState("0");
    const handleAmountChange = (event) => {
        const amount = event.target.value;
        setSelectedAmont(amount);
        onAmountChange(amount);
    };

    return (
        <>
            <h2>Unesite količinu (max 800g):</h2>
            <input
                id='amount'
                type="number"
                value={selectedAmount}
                onChange={handleAmountChange}
                min={0}
                max={800}
            />

            <p>Odabrana vrednost: {selectedAmount}g</p>
        </>
    );
};

export default Kolicina;
