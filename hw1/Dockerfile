# Use an official GCC image as the base
FROM gcc:latest

# Install make
RUN apt-get update && apt-get install -y make

# Set the working directory
WORKDIR /usr/src/app

# Copy the current directory contents into the container
COPY . .

# Install Composer if required
RUN apt-get update && apt-get install -y composer

# Run make clean and compile vm only
CMD ["make", "clean", "&&", "make", "vm"]
