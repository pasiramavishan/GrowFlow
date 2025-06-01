import React, { useState } from 'react';
import axios from 'axios';

const OrderForm = () => {
  const [userEmail, setUserEmail] = useState('');
  const [item, setItem] = useState('');
  const [paymentAmount, setPaymentAmount] = useState('');
  const [paymentType, setPaymentType] = useState('');
  const [paymentSlip, setPaymentSlip] = useState(null);

  const handleSubmit = async (e) => {
    e.preventDefault();

    const formData = new FormData();
    formData.append('userEmail', userEmail);
    formData.append('item', item);
    formData.append('paymentAmount', paymentAmount);
    formData.append('paymentType', paymentType);
    formData.append('paymentSlip', paymentSlip);

    try {
      const response = await axios.post('http://localhost:5000/api/orders', formData, {
        headers: {
          'Content-Type': 'multipart/form-data',
        },
      });
      console.log('Order created:', response.data);
      alert('Order submitted successfully!');
    } catch (error) {
      console.error('Error submitting order:', error);
      alert('Error submitting order. Please try again.');
    }
  };

  return (
    <form onSubmit={handleSubmit}>
      <input
        type="email"
        placeholder="Email"
        value={userEmail}
        onChange={(e) => setUserEmail(e.target.value)}
        required
      />
      <input
        type="text"
        placeholder="Item"
        value={item}
        onChange={(e) => setItem(e.target.value)}
        required
      />
      <input
        type="number"
        placeholder="Payment Amount"
        value={paymentAmount}
        onChange={(e) => setPaymentAmount(e.target.value)}
        required
      />
      <select value={paymentType} onChange={(e) => setPaymentType(e.target.value)} required>
        <option value="">Select Payment Type</option>
        <option value="Credit Card">Credit Card</option>
        <option value="Debit Card">Debit Card</option>
        <option value="Net Banking">Net Banking</option>
      </select>
      <input
        type="file"
        onChange={(e) => setPaymentSlip(e.target.files[0])}
        required
      />
      <button type="submit">Submit Order</button>
    </form>
  );
};

export default OrderForm;