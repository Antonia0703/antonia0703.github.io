TuneGenerator();
function TuneGenerator() {
    const notesGenerator = new NotesGenerator();
    setInterval(() => {
        notesGenerator.playNote(3);
    }, 1200); // Ritmo più spazioso
    setInterval(() => {
        notesGenerator.playNote(0);
    }, 4600);
}
function NotesGenerator() {
    const context = new AudioContext();
    // :control_knobs: Creiamo un riverbero
    const reverb = context.createConvolver();
    generateReverbImpulse(context, reverb);
    // :control_knobs: Aggiungiamo un delay per spazialità
    const delay = context.createDelay();
    delay.delayTime.value = 0.3;
    // :musical_note: Note e pesi per selezionare le frequenze
    const nextNote = distribution({
        131:2, 147:1, 165:2, 175:1, 196:2, 220:1, 247:1,
        262:2, 294:1, 330:2, 349:1, 392:2, 440:1, 496:1
    });
    // :level_slider: Fattore d'ottava
    const nextFac = distribution({ '-1':2, '0':2, '1':2, '2':1 });
    this.playNote = function(maxFactor) {
        const carrier = context.createOscillator(); // Oscillatore principale
        const modulator = context.createOscillator(); // Oscillatore di modulazione
        const modGain = context.createGain(); // Controlla l'intensità della modulazione
        const gainNode = context.createGain();
        const filter = context.createBiquadFilter();
        // :level_slider: Selezione randomizzata della forma d'onda per suoni organici
        carrier.type = Math.random() > 0.6 ? "sine" : "triangle";
        modulator.type = "sine";
        let fac = parseInt(nextFac());
        while (fac > maxFactor) fac = parseInt(nextFac());
        const freq = parseInt(nextNote()) * Math.pow(2, fac);
        carrier.frequency.setValueAtTime(freq, context.currentTime);
        // :control_knobs: Modulazione della frequenza per dare vita al suono
        modulator.frequency.value = Math.random() * 2 + 0.5; // Frequenza di vibrazione
        modGain.gain.value = Math.random() * 10 + 5; // Intensità della modulazione
        // :control_knobs: Filtro passa-basso per suoni più dolci
        filter.type = "lowpass";
        filter.frequency.setValueAtTime(freq * 1.5, context.currentTime);
        // :level_slider: Modelliamo il volume per una dissolvenza fluida
        gainNode.gain.setValueAtTime(0.3, context.currentTime);
        gainNode.gain.exponentialRampToValueAtTime(0.001, context.currentTime + 4);
        // :herb: Collegamenti tra i nodi audio
        modulator.connect(modGain);
        modGain.connect(carrier.frequency);
        carrier.connect(filter);
        filter.connect(gainNode);
        gainNode.connect(reverb);
        reverb.connect(delay);
        delay.connect(context.destination);
        // :musical_score: Avviamo la riproduzione
        modulator.start();
        carrier.start();
        carrier.stop(context.currentTime + 4);
        modulator.stop(context.currentTime + 4);
    }
    // :control_knobs: Generazione della risposta all'impulso per il riverbero
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
    function distribution(obj) {
        const o = normalizedObj(obj);
        return function() {
            let count = 0;
            const rand = Math.random();
            for (let key in o) {
                count += o[key];
                if (rand < count) return key;
            }
        }
    }
    function normalizedObj(obj) {
        let sum = Object.values(obj).reduce((a, b) => a + b, 0);
        return Object.fromEntries(Object.entries(obj).map(([k, v]) => [k, v / sum]));
    }
}

