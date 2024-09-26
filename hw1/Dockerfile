# Use an official GCC image as the base
FROM gcc:latest

# Install make and other dependencies
RUN apt-get update && apt-get install -y \
    make \
    zip \
    flex \
    bison \
    vim \
    && apt-get clean

# Set the working directory
WORKDIR /usr/src/app

# Copy the current directory contents into the container
COPY . .

# Install Composer if required (modify based on your actual requirements)
RUN apt-get update && apt-get install -y composer

# Keep the container running
CMD tail -f /dev/null
