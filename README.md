PR-Paket: Web-UI Amp-Toggle (Frontend)

Zweck
- Fügt eine kleine Frontend-Komponente hinzu, die per HTTP an einen Backend-Endpoint `/api/amp` einen AMP-Toggle sendet.
- Backend-Integration (Server) ist absichtlich nicht erzwungen — es gibt Beispiele für Node/Flask/C zum Einbauen.

Was ist enthalten
- `html/amp-toggle.js`  → UI-Logik für Button und Fetch-Requests
- `install_instructions.txt` → Snippets, wie du den Button in `html/index.html` einbindest und Beispiele für Server-Handlers

Wie anwenden
1) Kopiere `html/amp-toggle.js` nach `html/` in deinem Repo.
2) Ergänze in `html/index.html` an passender Stelle (z. B. in Controls) folgendes:

    <div id="amp-control"></div>
    <script src="amp-toggle.js"></script>

3) Implementiere `/api/amp` auf dem Gerät (siehe Beispiele unten). Endpoint: POST JSON { on: 0|1, persist: 0|1 }

Backend-Beispiele (Kurzversion)
- Node (express):

    const express = require('express');
    const bodyParser = require('body-parser');
    const { exec } = require('child_process');
    const app = express();
    app.use(bodyParser.json());

    app.post('/api/amp', (req,res)=>{
      const on = req.body.on ? 1 : 0;
      const persist = req.body.persist ? 1 : 0;
      if(persist){
        // Beispiel: nvs tool aufrufen, falls verfügbar
        exec(`nvs_set set_GPIO "21=amp"`, (err,out)=>{
          if(err) return res.status(500).send(err.message);
          // zusätzlich runtime toggle
          exec(`gpioctl 21 ${on}`,(e,o)=>{
            if(e) return res.status(500).send(e.message);
            res.json({ok:true});
          });
        });
      } else {
        exec(`gpioctl 21 ${on}`,(e,o)=>{
          if(e) return res.status(500).send(e.message);
          res.json({ok:true});
        });
      }
    });

- Python (Flask):

    from flask import Flask, request, jsonify
    import subprocess
    app = Flask(__name__)

    @app.post('/api/amp')
    def amp():
        j = request.json or {}
        on = 1 if j.get('on') else 0
        persist = j.get('persist',0)
        if persist:
            subprocess.run(['nvs_set','set_GPIO','21=amp'])
        subprocess.run(['gpioctl','21',str(on)])
        return jsonify(ok=True)

- Hinweis: Die obigen Aufrufe (`nvs_set`, `gpioctl`) sind Platzhalter; passe sie an die tatsächlichen Utilities/Methoden in deinem Image an.

Tests
- Nach Einbau: Browser öffnen → Button drücken → prüfe Logs / gpio-Level auf Gerät.

Wenn du möchtest, erstelle ich daraus ein Git-Branch-PR-Paket mit einem tatsächlichen Patch (branch + diff) — ich benötige dann Lesezugriff auf das Repo oder du überträgst die Dateien ins Repo und ich liefere den genauen commit/diff-Text.