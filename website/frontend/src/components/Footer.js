import React from 'react';
import { Link } from 'react-router-dom';

const Footer = () => {
  return (
    <footer id="footer" className="footer dark-background">
      <div className="footer-top">
        <div className="container">
          <div className="row gy-4">
            <div className="col-lg-6 col-md-6 footer-about">
              <Link to="/" className="logo d-flex align-items-center">
                <span className="sitename">AgriGenix</span>
              </Link>
              <div className="footer-contact pt-3">
                <p className="mt-3"><strong>Phone:</strong> <span>+94 76 281 1062</span></p>
                <p><strong>Email:</strong> <span>agrigenix@gmail.com</span></p>
                <p><strong>Address:</strong> <span>390/F, Kiriwaththuduwa Rd, Kahathuduwa, Gonapola Junction</span></p>
              </div>
            </div>
            <div className="col-lg-6 col-md-6 footer-links">
              <h4>Useful Links</h4>
              <ul>
                <li><Link to="/">Home</Link></li>
                <li><Link to="/about">About Us</Link></li>
                <li><Link to="/services">Services</Link></li>
                <li><Link to="#">Terms of Service</Link></li>
                <li><Link to="#">Privacy Policy</Link></li>
              </ul>
            </div>
          </div>
        </div>
      </div>
      <div className="copyright text-center">
        <div className="container d-flex flex-column flex-lg-row justify-content-center justify-content-lg-between align-items-center">
          <div className="d-flex flex-column align-items-center align-items-lg-start">
            <div>© Copyright <strong><span>GrowFlow</span></strong>. All Rights Reserved</div>
            <div className="credits">Designed by AgriGenix</div>
          </div>
          <div className="social-links order-first order-lg-last mb-3 mb-lg-0">
            <a href="#"><i className="bi bi-twitter-x"></i></a>
            <a href="#"><i className="bi bi-facebook"></i></a>
            <a href="#"><i className="bi bi-instagram"></i></a>
            <a href="#"><i className="bi bi-linkedin"></i></a>
          </div>
        </div>
      </div>
    </footer>
  );
};

export default Footer;