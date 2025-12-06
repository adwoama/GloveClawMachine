# EdgeImpulse model
https://studio.edgeimpulse.com/public/845392/live
**Processing Blocks:**
- IMU + Magnetometer → Spectral Features
- Flex → Raw Data Windowing + Statistical Features
- EMG → Spectral Features

**Learning Blocks:**
- Start with MLP (Fully Connected NN) for proof-of-concept (fast, small). 
- Experiment with 1D CNN or TCN for richer temporal modeling.
- Quantize to INT8 for deployment on microcontrollers.
