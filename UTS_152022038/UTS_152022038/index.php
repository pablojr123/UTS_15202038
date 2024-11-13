<?php
// Database connection settings
$servername = "localhost";
$username = "root"; // Change this according to your database username
$password = ""; // Change this according to your database password
$dbname = "uts_152022038"; // Database name

// Create a connection to the database
$conn = new mysqli($servername, $username, $password, $dbname);

// Check the connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// URL of the Python Flask API
$api_url = "http://localhost:5000/mqtt_data";

// Fetch data from the API
$response = file_get_contents($api_url);
$data = json_decode($response, true);

// Store data in the database
if ($data) {
    $timestamp = date("Y-m-d H:i:s"); // Current time when the data is received
    $temperature = $data['temperature'] ?? null;
    $humidity = $data['humidity'] ?? null;

    // Insert the data into the database table "temphum"
    $sql = "INSERT INTO temphum (timestamp, temperature, humidity) VALUES ('$timestamp', '$temperature', '$humidity')";

    if ($conn->query($sql) === TRUE) {
        echo "<p class='success-message'>DATA TERSIMPAN.</p>";
    } else {
        echo "<p class='error-message'>Error saving data: " . $conn->error . "</p>";
    }
} else {
    echo "<p class='error-message'>No data available from MQTT</p>";
}

// Retrieve all data from the database
$sql_select = "SELECT * FROM temphum ORDER BY timestamp DESC";
$result = $conn->query($sql_select);

// Display the latest data in a table
echo "<h1 class='title'>DATA TERBARU</h1>";
echo "<table class='data-table'>";
echo "<tr><th>Timestamp</th><th>Temperature</th><th>Humidity</th></tr>";

// Loop through all rows in the result and display them
if ($result->num_rows > 0) {
    while ($row = $result->fetch_assoc()) {
        echo "<tr><td>" . $row['timestamp'] . "</td><td>" . $row['temperature'] . "</td><td>" . $row['humidity'] . "</td></tr>";
    }
} else {
    echo "<tr><td colspan='3'>No data found</td></tr>";
}

echo "</table>";

// Close the database connection
$conn->close();
?>

<!-- Styling -->
<style>
    body {
        font-family: Arial, sans-serif;
        background-color: #f4f4f9;
        color: #333;
        margin: 0;
        padding: 0;
    }

    .container {
        width: 80%;
        margin: 50px auto;
        padding: 20px;
        background-color: #fff;
        border-radius: 10px;
        box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
    }

    .title {
        text-align: center;
        font-size: 24px;
        color: #4CAF50;
        margin-bottom: 20px;
    }

    .data-table {
        width: 100%;
        border-collapse: collapse;
        margin: 20px 0;
    }

    .data-table th, .data-table td {
        padding: 12px;
        text-align: center;
        border: 1px solid #ddd;
    }

    .data-table th {
        background-color: #4CAF50;
        color: white;
    }

    .data-table tr:nth-child(even) {
        background-color: #f2f2f2;
    }

    .success-message {
        color: green;
        font-weight: bold;
        text-align: center;
        margin: 10px 0;
    }

    .error-message {
        color: red;
        font-weight: bold;
        text-align: center;
        margin: 10px 0;
    }

    /* Add some responsiveness */
    @media (max-width: 768px) {
        .container {
            width: 90%;
        }
        
        .data-table th, .data-table td {
            font-size: 14px;
        }
    }
</style>

<!-- Optional: Add a container div -->
<div class="container">
    <!-- Content goes here (HTML and PHP output) -->
</div>
