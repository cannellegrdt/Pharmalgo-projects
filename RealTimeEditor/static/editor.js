
'use strict';

const CELL = 24;
const GAP = 2;
const STRIDE = CELL + GAP;
const PGAP = 10;
const PSIZ = 8 * STRIDE - GAP;
const PSTRIDE = PSIZ + PGAP;
const PAD = 26;

const CW = PAD * 2 + 3 * PSIZ + 2 * PGAP;
const CH = CW;

const PANEL_GRID = {
  top: [0, 1],
  left: [1, 0],
  center: [1, 1],
  right: [1, 2],
  bottom: [2, 1],
};
const PANEL_NAMES = ['top', 'left', 'center', 'right', 'bottom'];

const facePixels = { recto: {}, verso: {} };
for (const face of ['recto', 'verso']) {
  for (const name of PANEL_NAMES) {
    facePixels[face][name] = Array.from({ length: 8 }, () => new Uint8Array(8));
  }
}

let activeFace = 'recto';
let pixels = facePixels[activeFace];

let activeTool = 'pencil';
let activeBrightness = 7;
let mouseDown = false;
let autoSend = true;
let sendTimer = null;

const canvas = document.getElementById('canvas');
const ctx = canvas.getContext('2d');
const statusDot = document.getElementById('status-dot');
const statusTxt = document.getElementById('status-text');

const dpr = window.devicePixelRatio || 1;
canvas.width = Math.round(CW * dpr);
canvas.height = Math.round(CH * dpr);
canvas.style.width = CW + 'px';
canvas.style.height = CH + 'px';
ctx.scale(dpr, dpr);

function panelOrigin(name) {
  const [gr, gc] = PANEL_GRID[name];
  return { x: PAD + gc * PSTRIDE, y: PAD + gr * PSTRIDE };
}

function hitTest(cx, cy) {
  for (const name of PANEL_NAMES) {
    const { x: px, y: py } = panelOrigin(name);
    const lx = cx - px;
    const ly = cy - py;
    if (lx < 0 || ly < 0) continue;
    const col = Math.floor(lx / STRIDE);
    const row = Math.floor(ly / STRIDE);
    if (col >= 8 || row >= 8) continue;
    if ((lx - col * STRIDE) >= CELL || (ly - row * STRIDE) >= CELL) continue;
    return { panel: name, row, col };
  }
  return null;
}

function canvasPos(e) {
  const r = canvas.getBoundingClientRect();
  return {
    x: (e.clientX - r.left) * (CW / r.width),
    y: (e.clientY - r.top)  * (CH / r.height),
  };
}

function ledRGB(b) {
  const t = b / 7;
  return [Math.round(t * 20), Math.round(30 + t * 180), Math.round(t * 20)];
}
function ledCss(b) {
  if (b === 0) return '#091409';
  const [r, g, bv] = ledRGB(b);
  return `rgb(${r},${g},${bv})`;
}

function render() {
  ctx.clearRect(0, 0, CW, CH);

  ctx.fillStyle = '#0d0d1a';
  ctx.fillRect(0, 0, CW, CH);

  for (const [gr, gc] of [[0,0],[0,2],[2,0],[2,2]]) {
    const x = PAD + gc * PSTRIDE;
    const y = PAD + gr * PSTRIDE;
    ctx.fillStyle = '#0b0b14';
    ctx.fillRect(x - 4, y - 4, PSIZ + 8, PSIZ + 8);
  }

  for (const name of PANEL_NAMES) {
    const { x: px, y: py } = panelOrigin(name);

    ctx.fillStyle = '#11111e';
    roundRect(ctx, px - 5, py - 5, PSIZ + 10, PSIZ + 10, 7);
    ctx.fill();
    ctx.strokeStyle = '#252545';
    ctx.lineWidth = 1;
    roundRect(ctx, px - 5, py - 5, PSIZ + 10, PSIZ + 10, 7);
    ctx.stroke();

    ctx.fillStyle = '#303060';
    ctx.font = 'bold 9px monospace';
    ctx.textAlign = 'center';
    ctx.fillText(name.toUpperCase(), px + PSIZ / 2, py - 10);

    for (let r = 0; r < 8; r++) {
      for (let c = 0; c < 8; c++) {
        const b = pixels[name][r][c];
        const lx = px + c * STRIDE + CELL / 2;
        const ly = py + r * STRIDE + CELL / 2;
        const rad = CELL / 2 - 1;

        ctx.fillStyle = '#060e06';
        ctx.beginPath();
        ctx.arc(lx, ly, rad + 1.5, 0, Math.PI * 2);
        ctx.fill();

        if (b === 0) {
          ctx.fillStyle = '#0d180d';
          ctx.beginPath();
          ctx.arc(lx, ly, rad, 0, Math.PI * 2);
          ctx.fill();
        } else {
          const color = ledCss(b);

          if (b >= 4) {
            ctx.save();
            ctx.shadowColor = color;
            ctx.shadowBlur = 4 + (b - 3) * 4;
            ctx.fillStyle = color;
            ctx.beginPath();
            ctx.arc(lx, ly, rad, 0, Math.PI * 2);
            ctx.fill();
            ctx.restore();
          } else {
            ctx.fillStyle = color;
            ctx.beginPath();
            ctx.arc(lx, ly, rad, 0, Math.PI * 2);
            ctx.fill();
          }

          const grad = ctx.createRadialGradient(
            lx - rad * 0.3, ly - rad * 0.3, 0,
            lx, ly, rad
          );
          grad.addColorStop(0, 'rgba(255,255,255,0.22)');
          grad.addColorStop(1, 'rgba(255,255,255,0)');
          ctx.fillStyle = grad;
          ctx.beginPath();
          ctx.arc(lx, ly, rad, 0, Math.PI * 2);
          ctx.fill();
        }
      }
    }
  }
}

function roundRect(c, x, y, w, h, r) {
  c.beginPath();
  c.moveTo(x + r, y);
  c.lineTo(x + w - r, y);
  c.arcTo(x + w, y, x + w, y + r, r);
  c.lineTo(x + w, y + h - r);
  c.arcTo(x + w, y + h, x + w - r, y + h, r);
  c.lineTo(x + r, y + h);
  c.arcTo(x, y + h, x, y + h - r, r);
  c.lineTo(x, y + r);
  c.arcTo(x, y, x + r, y, r);
  c.closePath();
}

function applyTool(panel, row, col) {
  switch (activeTool) {
    case 'pencil':
      pixels[panel][row][col] = activeBrightness;
      break;
    case 'eraser':
      pixels[panel][row][col] = 0;
      break;
    case 'fill': {
      const target = pixels[panel][row][col];
      if (target !== activeBrightness) floodFill(panel, row, col, target, activeBrightness);
      break;
    }
    case 'eyedropper':
      setBrightness(pixels[panel][row][col]);
      setTool('pencil');
      break;
  }
}

function floodFill(panelName, sr, sc, target, fill) {
  const panel = pixels[panelName];
  const visited = new Uint8Array(64);
  const stack = [[sr, sc]];
  while (stack.length) {
    const [r, c] = stack.pop();
    if (r < 0 || r > 7 || c < 0 || c > 7) continue;
    const idx = r * 8 + c;
    if (visited[idx] || panel[r][c] !== target) continue;
    visited[idx] = 1;
    panel[r][c] = fill;
    stack.push([r-1,c],[r+1,c],[r,c-1],[r,c+1]);
  }
}

function setTool(tool) {
  activeTool = tool;
  document.querySelectorAll('.tool-btn[data-tool]').forEach(btn =>
    btn.classList.toggle('active', btn.dataset.tool === tool)
  );
  const hints = {
    pencil: 'Click and drag to draw',
    eraser: 'Click and drag to erase',
    fill: 'Click to flood fill',
    eyedropper: 'Click a LED to pick its brightness',
  };
  document.getElementById('cursor-hint').textContent = hints[tool] || '';
}

function setBrightness(b) {
  activeBrightness = b;
  document.getElementById('bright-slider').value = b;
  document.getElementById('bright-num').textContent = b;

  const color = ledCss(b);
  const preview = document.getElementById('bright-preview');
  preview.style.background = color;
  preview.style.boxShadow = b >= 3 ? `0 0 ${(b - 2) * 7}px ${color}` : 'none';
  preview.style.borderColor = b > 0  ? color : 'var(--border)';

  document.querySelectorAll('.bswatch').forEach(s =>
    s.classList.toggle('active', +s.dataset.b === b)
  );
}

function switchFace(face) {
  activeFace = face;
  pixels = facePixels[face];

  document.querySelectorAll('.face-tab').forEach(btn =>
    btn.classList.toggle('active', btn.dataset.face === face)
  );

  const indicator = document.getElementById('face-indicator');
  indicator.textContent = face.toUpperCase();
  indicator.className = 'face-indicator ' + face;

  render();
}

function updateCursorInfo(hit) {
  if (hit) {
    document.getElementById('i-panel').textContent = hit.panel;
    document.getElementById('i-row').textContent = hit.row;
    document.getElementById('i-col').textContent = hit.col;
    document.getElementById('i-val').textContent = pixels[hit.panel][hit.row][hit.col];
  } else {
    ['i-panel','i-row','i-col','i-val'].forEach(id =>
      document.getElementById(id).textContent = '—'
    );
  }
}

function setStatus(state) {
  statusDot.className = 'status-dot ' + state;
  statusTxt.textContent = { sending: 'Sending…', ok: 'Sent', error: 'Simulator not running', '': 'Ready' }[state] ?? state;
}

function getPanelsPayload(face) {
  const panels = {};
  for (const name of PANEL_NAMES) {
    panels[name] = Array.from(facePixels[face][name], row => Array.from(row));
  }
  return panels;
}

async function doSend() {
  sendTimer = null;
  setStatus('sending');
  try {
    const res = await fetch('/api/frame', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ face: activeFace, panels: getPanelsPayload(activeFace) }),
    });
    setStatus(res.ok ? 'ok' : 'error');
  } catch {
    setStatus('error');
  }
}

async function doSendBoth() {
  setStatus('sending');
  try {
    const res = await fetch('/api/frame/both', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        faces: {
          recto: getPanelsPayload('recto'),
          verso: getPanelsPayload('verso'),
        },
      }),
    });
    setStatus(res.ok ? 'ok' : 'error');
  } catch {
    setStatus('error');
  }
}

function scheduleSend() {
  if (!autoSend) return;
  if (sendTimer) clearTimeout(sendTimer);
  sendTimer = setTimeout(doSend, 50);
}

function handleDown(e) {
  mouseDown = true;
  const hit = hitTest(canvasPos(e).x, canvasPos(e).y);
  if (hit) { applyTool(hit.panel, hit.row, hit.col); render(); scheduleSend(); }
}

function handleMove(e) {
  const pos = canvasPos(e);
  const hit = hitTest(pos.x, pos.y);
  updateCursorInfo(hit);
  if (mouseDown && (activeTool === 'pencil' || activeTool === 'eraser') && hit) {
    applyTool(hit.panel, hit.row, hit.col);
    render();
    scheduleSend();
  }
}

function handleUp() {
  mouseDown = false;
}

canvas.addEventListener('mousedown', handleDown);
canvas.addEventListener('mousemove', handleMove);
canvas.addEventListener('mouseup',   handleUp);
canvas.addEventListener('mouseleave', handleUp);

canvas.addEventListener('touchstart', e => {
  e.preventDefault();
  handleDown(e.touches[0]);
}, { passive: false });

canvas.addEventListener('touchmove', e => {
  e.preventDefault();
  handleMove(e.touches[0]);
}, { passive: false });

canvas.addEventListener('touchend', () => mouseDown = false);

document.querySelectorAll('.tool-btn[data-tool]').forEach(btn =>
  btn.addEventListener('click', () => setTool(btn.dataset.tool))
);

document.querySelectorAll('.face-tab').forEach(btn =>
  btn.addEventListener('click', () => switchFace(btn.dataset.face))
);

document.getElementById('bright-slider').addEventListener('input', e =>
  setBrightness(+e.target.value)
);

document.getElementById('autoSend').addEventListener('change', e => {
  autoSend = e.target.checked;
});

document.getElementById('sendBtn').addEventListener('click', doSend);
document.getElementById('sendBothBtn').addEventListener('click', doSendBoth);

document.getElementById('clearAllBtn').addEventListener('click', () => {
  for (const name of PANEL_NAMES) pixels[name].forEach(row => row.fill(0));
  render();
  scheduleSend();
});

document.addEventListener('keydown', e => {
  if (e.target.tagName === 'INPUT') return;
  switch (e.key) {
    case 'p': case 'P': setTool('pencil'); break;
    case 'e': case 'E': setTool('eraser'); break;
    case 'f': case 'F': setTool('fill'); break;
    case 'i': case 'I': setTool('eyedropper'); break;
    case 's': case 'S': doSend(); break;
    case 'b': case 'B': doSendBoth(); break;
    case 'Tab':
      e.preventDefault();
      switchFace(activeFace === 'recto' ? 'verso' : 'recto');
      break;
    case 'Delete': case 'Backspace':
      for (const name of PANEL_NAMES) pixels[name].forEach(row => row.fill(0));
      render();
      scheduleSend();
      break;
  }
  if (e.key >= '0' && e.key <= '7') setBrightness(+e.key);
});

(function buildSwatches() {
  const container = document.getElementById('bswatches');
  for (let b = 0; b <= 7; b++) {
    const el = document.createElement('div');
    el.className = 'bswatch' + (b === activeBrightness ? ' active' : '');
    el.dataset.b = b;
    el.title = `Brightness ${b}`;
    el.style.background = ledCss(b);
    if (b >= 4) el.style.boxShadow = `0 0 ${b}px ${ledCss(b)}`;
    el.addEventListener('click', () => setBrightness(b));
    container.appendChild(el);
  }
})();

setBrightness(7);
setTool('pencil');
switchFace('recto');
render();
