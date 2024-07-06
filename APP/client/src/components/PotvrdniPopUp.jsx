import Modal from 'react-modal';
import './PotvrdniPopUp.css';
const PotvrdniPopUp = ({ isOpen, onRequestClose, onConfirm, kolicina, ponavljanje, vreme }) => {
    return (
        <Modal className='modal-content'
            isOpen={isOpen}
            onRequestClose={onRequestClose}
            contentLabel="Potvrda"
        >
            <h2>Da li ste sigurni?</h2>
            <h3>Odabrali ste:</h3>
            <label>Kolicina: {kolicina}g</label>
            <label>Vreme izmedju obroka: {ponavljanje}h</label>
            <label>Vreme prvog obroka: {vreme}h</label>
            <div className='buttons'>
                <button className='confirm-button' onClick={onConfirm}>Nahrani</button>
                <button className='cancel-button' onClick={onRequestClose}>Odustani</button>
            </div>
        </Modal>
    );
};

export default PotvrdniPopUp;