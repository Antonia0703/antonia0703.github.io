// Supabase credentials
const SUPABASE_URL = "https://bhuvkendkwmmnbzkjxsr.supabase.co";
const SUPABASE_API_KEY = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImJodXZrZW5ka3dtbW5iemtqeHNyIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NDExNzY3OTUsImV4cCI6MjA1Njc1Mjc5NX0.EKYOtg_1rExta0k4UkZGViJg3bup-a3uRrZI1Ih0bzM"; // Replace with your actual anon key
const TABLE_NAME = "sensors_values";
const DEVICE_ID = "plant_1";

const db = supabase.createClient(SUPABASE_URL, SUPABASE_API_KEY);


// Global variable to store sensor values for sound generation
let sensorSoundData = {
    moisture: 1,    // default moisture value (0: dry, 1: good, 2: wet)
    movement: false // default movement (false = no movement, true = movement)
  };
  
  
  async function fetchSensorData(screenNumber = 1) {
    try {
      let { data, error } = await db
        .from(TABLE_NAME)
        .select("values")
        .eq("device_id", DEVICE_ID)
        .order("created_at", { ascending: false })
        .limit(1);
  
      if (error) throw error;
  
      if (data.length > 0) {
        const sensorData = data[0].values;
        updateImage(screenNumber, sensorData.moisture, sensorData.movement);
        // Update sensor values for sound generation.
        sensorSoundData.moisture = sensorData.moisture;
        sensorSoundData.movement = sensorData.movement;
      } else {
        console.log("No data available.");
      }
    } catch (err) {
      console.error("Error fetching sensor data:", err);
    }
  }
  

  function getOrientation() {
    const width = window.innerWidth;
    const height = window.innerHeight;
  
    if (width > height) {
      return "h"; // Landscape 
    } else {
      return "v"; // Portrait 
    }
  }
  
  
  let numbers = [];
  function getUniqueRandomNumber(max) {
    if (numbers.length === 0) {
      for (let i = 1; i <= max; i++) {
        numbers.push(i);
      }
      // Shuffle numbers randomly.
      for (let i = numbers.length - 1; i > 0; i--) {
        let j = Math.floor(Math.random() * (i + 1));
        [numbers[i], numbers[j]] = [numbers[j], numbers[i]];
      }
    }
    return numbers.pop();
  }
  
  async function updateImage(screenNumber, moisture, movement) {
    const moistureMap = ["dry", "good", "wet"];
    const moistureText = moistureMap[moisture] || "unknown";
    const movementStatus = movement ? "movement" : "no-movement";
    const orientation = getOrientation();
    const randomNumber = getUniqueRandomNumber(5);
    const imagePath = `plant/${moistureText}/${movementStatus}/0${randomNumber}_${orientation}.jpg`;
    document.getElementById("plantImage").src = imagePath;
  }
  
  // Refresh image every 2 seconds.
  refreshImage();
  setInterval(refreshImage, 2000);
  window.addEventListener("resize", refreshImage);
  function refreshImage() {
    fetchSensorData();
  }
  

  // SOUND GENERATION
  let audioCtx = new (window.AudioContext || window.webkitAudioContext)();
  
 
  let soundStarted = false;
  document.body.addEventListener("click", async function initSound() {
    if (audioCtx.state === "suspended") {
      await audioCtx.resume();
      console.log("AudioContext resumed after user gesture.");
    }
    if (!soundStarted) {
      TuneGenerator(audioCtx);
      soundStarted = true;
    }
    
    document.body.removeEventListener("click", initSound);
  });
  
  function TuneGenerator(context) {
    const notesGenerator = new NotesGenerator(context);
    setInterval(() => {
      notesGenerator.playNote(sensorSoundData.moisture, sensorSoundData.movement);
    }, 1200);
  }
  
  function NotesGenerator(context) {
  
    const reverb = context.createConvolver();
    generateReverbImpulse(context, reverb);
  
    // Delau for spatial effect
    const delay = context.createDelay();
    delay.delayTime.value = 0.3;
  
    const scaleMapping = {
      0: [220, 233, 246.94, 261.63, 277.18],                // Dry: A3, A#3, B3, C4, C#4
      1: [329.63, 349.23, 369.99, 392.00, 415.30, 440.00],    // Good: E4, F4, F#4, G4, G#4, A4
      2: [392.00, 415.30, 440.00, 466.16, 493.88, 523.25]     // Wet: G4, G#4, A4, A#4, B4, C5
    };
  
    // playNote chooses a note based on sensor values and creates the sound.
    this.playNote = function(moisture, movement) {
      const notes = scaleMapping[moisture] || scaleMapping[1];
      const freq = notes[Math.floor(Math.random() * notes.length)];
  

      const carrier = context.createOscillator();
      const modulator = context.createOscillator();
      const modGain = context.createGain();
      const gainNode = context.createGain();
      const filter = context.createBiquadFilter();
  
     
      carrier.type = "sine";
      modulator.type = "sine";
  
      modulator.frequency.value = movement ? 2.0 : 0.5;
      modGain.gain.value = movement ? 5 : 3;
  
      carrier.frequency.setValueAtTime(freq, context.currentTime);
  
   
      filter.type = "lowpass";
      filter.frequency.setValueAtTime(freq * 1.5, context.currentTime);
      filter.Q.value = 1.2;
  

      gainNode.gain.setValueAtTime(0.6, context.currentTime);
      gainNode.gain.exponentialRampToValueAtTime(0.05, context.currentTime + 4);
  

      modulator.connect(modGain);
      modGain.connect(carrier.frequency);
      carrier.connect(filter);
      filter.connect(gainNode);
      gainNode.connect(reverb);
      reverb.connect(delay);
      delay.connect(context.destination);
  

      modulator.start();
      carrier.start();
      carrier.stop(context.currentTime + 4);
      modulator.stop(context.currentTime + 4);
    };
  

    function generateReverbImpulse(ctx, convolver) {
      const length = ctx.sampleRate * 2;
      const impulse = ctx.createBuffer(2, length, ctx.sampleRate);
      const impulseL = impulse.getChannelData(0);
      const impulseR = impulse.getChannelData(1);
      for (let i = 0; i < length; i++) {
        const decay = Math.pow(1 - i / length, 2.5);
        impulseL[i] = (Math.random() * 2 - 1) * decay;
        impulseR[i] = (Math.random() * 2 - 1) * decay;
      }
      convolver.buffer = impulse;
    }
  }