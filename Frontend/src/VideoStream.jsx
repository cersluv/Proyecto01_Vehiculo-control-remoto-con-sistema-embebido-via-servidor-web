import { useState, useEffect, useRef } from 'react';

function VideoStream({ apiBase, credentials }) {
  const [frame, setFrame] = useState(null);
  const [streaming, setStreaming] = useState(false);
  const [fps, setFps] = useState(0);
  const intervalRef = useRef(null);
  const fpsCountRef = useRef(0);
  const lastFpsUpdate = useRef(Date.now());

  const fetchFrame = async () => {
    try {
      const response = await fetch(`${apiBase}/tomar_foto`, {
        headers: { Authorization: `Basic ${credentials}` }
      });
      
      if (response.ok) {
        const base64 = await response.text();
        setFrame(`data:image/jpeg;base64,${base64}`);
        
        // Calcular FPS
        fpsCountRef.current++;
        const now = Date.now();
        if (now - lastFpsUpdate.current >= 1000) {
          setFps(fpsCountRef.current);
          fpsCountRef.current = 0;
          lastFpsUpdate.current = now;
        }
      }
    } catch (error) {
      console.error('Error capturando frame:', error);
    }
  };

  const startStream = () => {
    setStreaming(true);
    // Tomar fotos cada 200ms = ~5 FPS (ajusta según necesites)
    intervalRef.current = setInterval(fetchFrame, 200);
  };

  const stopStream = () => {
    setStreaming(false);
    if (intervalRef.current) {
      clearInterval(intervalRef.current);
      intervalRef.current = null;
    }
    setFps(0);
    fpsCountRef.current = 0;
  };

  useEffect(() => {
    return () => {
      if (intervalRef.current) {
        clearInterval(intervalRef.current);
      }
    };
  }, []);

  return (
    <div style={styles.container}>
      <div style={styles.controls}>
        <button
          onClick={streaming ? stopStream : startStream}
          style={{
            ...styles.button,
            backgroundColor: streaming ? '#ff5555' : '#50fa7b'
          }}
        >
          {streaming ? '⏹️ Detener Stream' : '▶️ Iniciar Stream'}
        </button>
        {streaming && (
          <span style={styles.fpsIndicator}>
            📹 {fps} FPS
          </span>
        )}
      </div>

      <div style={styles.videoContainer}>
        {frame ? (
          <img 
            src={frame} 
            alt="Video stream" 
            style={styles.video}
          />
        ) : (
          <div style={styles.placeholder}>
            {streaming ? '⏳ Cargando stream...' : '📷 Presiona Iniciar Stream'}
          </div>
        )}
      </div>

      <div style={styles.warning}>
        ⚠️ Nota: somos demasiado pichudos para manejar streams en tiempo real. Este método simula un stream tomando fotos periódicamente.
      </div>
    </div>
  );
}

const styles = {
  container: {
    width: '100%',
    padding: '20px',
    backgroundColor: 'rgba(0, 0, 0, 0.3)',
    borderRadius: '10px'
  },
  controls: {
    display: 'flex',
    alignItems: 'center',
    gap: '15px',
    marginBottom: '15px'
  },
  button: {
    padding: '12px 24px',
    border: 'none',
    borderRadius: '8px',
    fontSize: '16px',
    fontWeight: 'bold',
    cursor: 'pointer',
    color: '#000',
    transition: '0.3s'
  },
  fpsIndicator: {
    color: '#50fa7b',
    fontWeight: 'bold',
    fontSize: '18px',
    backgroundColor: 'rgba(0, 0, 0, 0.5)',
    padding: '8px 16px',
    borderRadius: '20px'
  },
  videoContainer: {
    position: 'relative',
    width: '100%',
    backgroundColor: '#000',
    borderRadius: '8px',
    overflow: 'hidden',
    minHeight: '300px',
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center'
  },
  video: {
    width: '100%',
    height: 'auto',
    display: 'block'
  },
  placeholder: {
    color: '#888',
    fontSize: '20px',
    textAlign: 'center',
    padding: '40px'
  },
  warning: {
    marginTop: '15px',
    padding: '10px',
    backgroundColor: 'rgba(255, 120, 0, 0.2)',
    border: '1px solid rgba(255, 120, 0, 0.5)',
    borderRadius: '5px',
    color: '#ffaa00',
    fontSize: '14px',
    textAlign: 'center'
  }
};

export default VideoStream;