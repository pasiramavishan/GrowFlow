import React from 'react';
// import OrderForm from '../components/OrderForm';
// import OrderList from '../components/OrderList';

import OrderForm from '../components/OrderForm';
import OrderList from '../components/OrderList';

const OrderPage = () => {
  return (
    <div>
      <h1>Customer Order Section</h1>
      <OrderForm />
      <OrderList />
    </div>
  );
};

export default OrderPage;

