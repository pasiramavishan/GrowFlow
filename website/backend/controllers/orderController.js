const Order = require('../models/Order');
const multer = require('multer');

// Multer configuration for file uploads
const storage = multer.diskStorage({
  destination: (req, file, cb) => {
    cb(null, 'uploads/');
  },
  filename: (req, file, cb) => {
    cb(null, Date.now() + '-' + file.originalname);
  },
});

const upload = multer({ storage }).single('paymentSlip');

// Create a new order
const createOrder = (req, res) => {
  upload(req, res, (err) => {
    if (err) {
      return res.status(400).json({ message: 'File upload failed', error: err });
    }

    const { userEmail, item, paymentAmount, paymentType } = req.body;
    const paymentSlip = req.file ? req.file.path : '';

    const newOrder = new Order({
      userEmail,
      item,
      paymentAmount,
      paymentType,
      paymentSlip,
    });

    newOrder
      .save()
      .then((order) => res.status(201).json(order))
      .catch((error) => res.status(400).json({ message: 'Error creating order', error }));
  });
};

// Get all orders
const getOrders = (req, res) => {
  Order.find()
    .then((orders) => res.status(200).json(orders))
    .catch((error) => res.status(400).json({ message: 'Error fetching orders', error }));
};

module.exports = { createOrder, getOrders };