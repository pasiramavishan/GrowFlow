const express = require('express');
const { createOrder, getOrders } = require('../controllers/orderController');

const router = express.Router();

// Create a new order
router.post('/', createOrder);

// Get all orders
router.get('/', getOrders);

module.exports = router;