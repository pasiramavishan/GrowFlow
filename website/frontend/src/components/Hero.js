import React from 'react';

const Hero = () => {
  return (
    <section id="hero" className="hero section dark-background">
      <div id="hero-carousel" className="carousel slide carousel-fade" data-bs-ride="carousel" data-bs-interval="5000">
      <div class="carousel-item active">
          <img src="assets/img/hero_1.jpeg" alt=""/>
          <div class="carousel-container">
            <h2>Effortless Global Control </h2>
            <p>Our user-friendly app lets you monitor and control your plantation from anywhere in the world, giving you the freedom to manage your farm remotely without any barriers..</p>
            <a href="https://htmlcodex.com" class="btn-get-started">Download GrowFlow</a>
          </div>
        </div>
        {/* <!-- End Carousel Item --> */}

        <div class="carousel-item">
          <img src="assets/img/hero_2.png" alt=""/>
          <div class="carousel-container">
            <h2>Real-Time Plant Monitoring</h2>
            <p>Track key plant parameters like soil moisture, temperature, and humidity in real time to make data-driven decisions.</p>
            <a href="https://htmlcodex.com" class="btn-get-started">Download GrowFlow</a>
          </div>
        </div>
        {/* <!-- End Carousel Item --> */}

        <div class="carousel-item">
          <img src="assets/img/hero_4.png" alt=""/>
          <div class="carousel-container">
            <h2>Seamless Node Expansion</h2>
            <p>As your plantation grows, adding new nodes is quick and simple, allowing you to easily scale your farm without complexity.</p>
            <a href="https://htmlcodex.com" class="btn-get-started">Download GrowFlow</a>
          </div>
        </div>
        {/* <!-- End Carousel Item --> */}

        <div class="carousel-item">
          <img src="assets/img/hero_5.jpg" alt=""/>
          <div class="carousel-container">
            <h2>Precision Watering Control</h2>
            <p>Manage watering for individual nodes manually, or switch to auto mode to ensure your plants are watered even when you're not around.</p>
            <a href="https://htmlcodex.com" class="btn-get-started">Download GrowFlow</a>
          </div>
        </div>
        {/* <!-- End Carousel Item --> */}

        <div class="carousel-item">
          <img src="assets/img/hero_3.jpeg" alt=""/>
          <div class="carousel-container">
            <h2>Smarter, More Efficient Farming</h2>
            <p>Experience a smarter approach to farming with minimal effort, optimizing resources and ensuring your plants thrive in any condition.</p>
            <a href="https://htmlcodex.com" class="btn-get-started">Download GrowFlow</a>
          </div>
        </div>
        {/* <!-- End Carousel Item --> */}

        <a class="carousel-control-prev" href="#hero-carousel" role="button" data-bs-slide="prev">
          <span class="carousel-control-prev-icon bi bi-chevron-left" aria-hidden="true"></span>
        </a>

        <a class="carousel-control-next" href="#hero-carousel" role="button" data-bs-slide="next">
          <span class="carousel-control-next-icon bi bi-chevron-right" aria-hidden="true"></span>
        </a>

        <ol class="carousel-indicators"></ol>

      </div>
    </section>
  );
};

export default Hero;