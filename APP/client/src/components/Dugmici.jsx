import './Dugmici.css';
const Dugmici = ({ onButtonClick }) => {
    return (
        <>
            <h2>Odaberi razmak izmedju obroka:</h2>
            <div className="dugmici">
                <button onClick={() => onButtonClick(2)}>2 min</button>
                <button onClick={() => onButtonClick(8)}>8h</button>
                <button onClick={() => onButtonClick(12)}>12h</button>
                <button onClick={() => onButtonClick(24)}>24h</button>
            </div>
        </>
    )
}
export default Dugmici;