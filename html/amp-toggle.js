// amp-toggle.js
// Fügt einen Button in #amp-control ein und ruft /api/amp an.

(function(){
  function createUI(){
    const container = document.getElementById('amp-control');
    if(!container) return;

    const btn = document.createElement('button');
    btn.id = 'amp-toggle-btn';
    btn.textContent = 'AMP: Aus';
    btn.dataset.on = '0';

    const persistLabel = document.createElement('label');
    persistLabel.style.marginLeft = '8px';
    const chk = document.createElement('input');
    chk.type = 'checkbox';
    chk.id = 'amp-persist';
    persistLabel.appendChild(chk);
    persistLabel.appendChild(document.createTextNode(' Persistieren'));

    container.appendChild(btn);
    container.appendChild(persistLabel);

    btn.addEventListener('click', async ()=>{
      const on = btn.dataset.on === '1' ? 0 : 1;
      const persist = chk.checked ? 1 : 0;
      try{
        btn.disabled = true;
        const resp = await fetch('/api/amp', {
          method: 'POST',
          headers: {'Content-Type':'application/json'},
          body: JSON.stringify({on:on, persist:persist})
        });
        if(!resp.ok) throw new Error('HTTP '+resp.status);
        btn.dataset.on = String(on);
        btn.textContent = 'AMP: ' + (on? 'An':'Aus');
      }catch(e){
        console.error('AMP toggle failed', e);
        alert('AMP toggle failed: '+e.message);
      }finally{
        btn.disabled = false;
      }
    });
  }

  if(document.readyState === 'loading') document.addEventListener('DOMContentLoaded', createUI);
  else createUI();
})();