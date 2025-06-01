const mongoose = require('mongoose');

const OrderSchema = new mongoose.Schema({
  userEmail: {
    type: String,
    required: true,
  },
  item: {
    type: String,
    required: true,
  },
  paymentAmount: {
    type: Number,
    required: true,
  },
  paymentType: {
    type: String,
    required: true,
  },
  paymentSlip: {
    type: String, // Path to the uploaded image
    required: true,
  },
  createdAt: {
    type: Date,
    default: Date.now,
  },
});

module.exports = mongoose.model('Order', OrderSchema);