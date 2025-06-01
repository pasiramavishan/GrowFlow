import React, { useEffect, useState } from 'react';
import axios from 'axios';

const OrderList = () => {
  const [orders, setOrders] = useState([]);

  useEffect(() => {
    const fetchOrders = async () => {
      try {
        const response = await axios.get('http://localhost:5000/api/orders');
        setOrders(response.data);
      } catch (error) {
        console.error('Error fetching orders:', error);
      }
    };

    fetchOrders();
  }, []);

  return (
    <div>
      <h2>Orders</h2>
      {orders.map((order) => (
        <div key={order._id}>
          <p>Email: {order.userEmail}</p>
          <p>Item: {order.item}</p>
          <p>Amount: {order.paymentAmount}</p>
          <p>Payment Type: {order.paymentType}</p>
          <img
            src={`http://localhost:5000/uploads/${order.paymentSlip.split('uploads/')[1]}`}
            alt="Payment Slip"
            style={{ width: '200px' }}
          />
        </div>
      ))}
    </div>
  );
};

export default OrderList;