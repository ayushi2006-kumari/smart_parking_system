const express = require("express");
const { spawn } = require("child_process");
const fs = require("fs");
const path = require("path");
const cors = require("cors");

const app = express();

app.use(cors());
app.use(express.json());
app.use(express.static("public"));

// ---------------- ADMIN PASSWORD ----------------
const adminPasswordFile = path.join(__dirname, "adminPassword.txt");

app.post("/run", (req, res) => {
    const { input } = req.body;
    const executablePath = path.join(__dirname, "parking"); 
    const cppProgram = spawn(executablePath); 

    let output = "";

    
    cppProgram.stdin.write(input);
    cppProgram.stdin.end();

    cppProgram.stdout.on("data", (data) => {
        output += data.toString();
    });

    cppProgram.on("close", (code) => {
        res.send(output); 
    });
});

// ---------------- START SERVER ----------------
const PORT = 3000;
app.listen(PORT, () => console.log(`Server running on http://localhost:${PORT}`));