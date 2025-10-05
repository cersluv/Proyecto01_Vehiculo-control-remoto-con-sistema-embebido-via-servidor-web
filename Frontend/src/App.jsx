import React, { useState } from "react";
import { FaEye, FaEyeSlash, FaUser, FaLightbulb, FaCamera, FaSignOutAlt } from "react-icons/fa";
import VideoStream from "./VideoStream";

const API_BASE = "http://192.168.100.174:5000";

function App() {
  const [isLoggedIn, setIsLoggedIn] = useState(false);
  const [username, setUsername] = useState("");
  const [password, setPassword] = useState("");
  const [mostrarContrasena, setMostrarContrasena] = useState(false);
  const [authError, setAuthError] = useState("");
  const [credentials, setCredentials] = useState(null);
  const [cargandoFoto, setCargandoFoto] = useState(false);
  const [loading, setLoading] = useState(false);
  const [foto, setFoto] = useState(null);
  const [camara, setCamara] = useState("jardin");
  const [velocidad, setVelocidad] = useState(60);

const [estadoCarro, setEstadoCarro] = useState({
  luces: {
    delantera: false,
    traseras: false,
    izquierda: false,
    derecha: false
  },
  direccion: "Detenido"
});

  
  // Estados para feedback visual
  const [lucesActivas, setLucesActivas] = useState({
    delantera: false,
    traseras: false,
    izquierda: false,
    derecha: false
  });
  const [direccionActiva, setDireccionActiva] = useState("");


  const handleLogin = async (e) => {
    e.preventDefault();
    setAuthError("");
    setLoading(true);

    try {
      const token = btoa(`${username}:${password}`);

      // ✅ Intentar autenticación real usando /mover (no cambia nada físico)
      const response = await fetch(`${API_BASE}/mover`, {
        method: "POST",
        headers: {
          "Authorization": `Basic ${token}`,
          "Content-Type": "application/json"
        },
        body: JSON.stringify({ direccion: "detener", velocidad: 0 })
      });

      if (response.status === 401) {
        throw new Error("Credenciales inválidas");
      }

      if (!response.ok) {
        throw new Error(`Error HTTP ${response.status}`);
      }

      // ✅ Si pasa, guardar credenciales y permitir acceso
      setCredentials(token);
      setIsLoggedIn(true);
      setAuthError("");
      console.log("🔒 Login exitoso");
    } catch (error) {
      console.error("Error autenticando:", error);
      setAuthError("Usuario o contraseña incorrectos");
      setIsLoggedIn(false);
    } finally {
      setLoading(false);
    }
  };




  const authFetch = async (url, options = {}) => {
    if (!credentials) throw new Error("No hay credenciales guardadas");
    
    const headers = {
      ...options.headers,
      Authorization: `Basic ${credentials}`
    };

    const response = await fetch(url, { ...options, headers });
    if (!response.ok) {
      if (response.status === 401) {
        throw new Error("Acceso no autorizado. Verifica tus credenciales.");
      } else {
        throw new Error(`Error HTTP ${response.status}`);
      }
    }
    return response;
  };


  const manejarLuz = async (tipo) => {
    console.log(`💡 Enviando comando de luz: ${tipo}`);

    // Toggle visual inmediato
    setLucesActivas(prev => ({
      ...prev,
      [tipo]: !prev[tipo]
    }));

    try {
      const response = await authFetch(`${API_BASE}/luz`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ tipo })
      });
      const data = await response.json();
      console.log("Respuesta del servidor:", data);

      // Actualizar estadoCarro con luces individuales
      setEstadoCarro(prev => ({
        ...prev,
        luces: {
          ...prev.luces,
          [tipo]: !prev.luces[tipo]
        }
      }));
    } catch (err) {
      console.error("Error en manejarLuz:", err);
      // Revertir si falla
      setLucesActivas(prev => ({
        ...prev,
        [tipo]: !prev[tipo]
      }));
    }
  };


  const moverCarroConDireccion = async (direccion) => {
    console.log(`🚗 Enviando comando de movimiento: ${direccion}, vel=${velocidad}%`);
    
    // Activar feedback visual
    setDireccionActiva(direccion);
    
    try {
      const response = await authFetch(`${API_BASE}/mover`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ direccion, velocidad })
      });
      const data = await response.json();
      console.log("Respuesta del servidor:", data);
      setEstadoCarro((prev) => ({ ...prev, direccion }));
      
      // Mantener activo por 1 segundo
      setTimeout(() => setDireccionActiva(""), 1000);
    } catch (error) {
      console.error("Error moviendo carro:", error);
      setDireccionActiva("");
    }
  };

// === Detener movimiento ===
const detenerCarro = async () => {
    console.log("🛑 Enviando comando para DETENER el carro");

    try {
      const response = await authFetch(`${API_BASE}/detener`, {
        method: "POST", // o GET si tu servidor no usa método
        headers: { "Content-Type": "application/json" }
      });

      const data = await response.json();
      console.log("Respuesta del servidor:", data);

      // Actualiza el estado del carro
      setEstadoCarro(prev => ({
        ...prev,
        direccion: "Detenido"
      }));
      setDireccionActiva("");
    } catch (error) {
      console.error("Error deteniendo el carro:", error);
    }
  };

  const cambiarVelocidad = () => {
    setVelocidad((prev) => {
      const nueva = prev === 40 ? 60 
                  : prev === 60 ? 80 
                  : prev === 80 ? 90 
                  : 40;
      console.log(`⚙️ Nueva velocidad: ${nueva}%`);
      return nueva;
    });
  };

  const tomarFoto = async () => {
    setCargandoFoto(true);
    setFoto(null);

    try {
      const response = await authFetch(`${API_BASE}/tomar_foto?camara=${camara}`);
      
      if (!response.ok) {
        throw new Error(`Error: ${response.status}`);
      }
      
      const encodedData = await response.text();
      const imageUrl = `data:image/jpeg;base64,${encodedData}`;
      setFoto(imageUrl);
      
    } catch (error) {
      console.error("Error tomando la foto:", error);
      setFoto("error");
    } finally {
      setCargandoFoto(false);
    }
  };

  const handleLogout = () => {
    setIsLoggedIn(false);
    setCredentials(null);
    setUsername("");
    setPassword("");
    setLucesActivas({
      delantera: false,
      traseras: false,
      izquierda: false,
      derecha: false
    });
    setDireccionActiva("");
  };

  // Función helper para obtener estilo de botón de luz
  const getLuzButtonStyle = (tipo) => ({
    ...styles.controlButton,
    backgroundColor: lucesActivas[tipo] ? '#50fa7b' : '#e0e0e0',
    color: lucesActivas[tipo] ? '#000' : '#333',
    fontWeight: lucesActivas[tipo] ? 'bold' : 'normal',
    transform: lucesActivas[tipo] ? 'scale(1.05)' : 'scale(1)',
    boxShadow: lucesActivas[tipo] 
      ? '0 0 20px rgba(80, 250, 123, 0.6)' 
      : '0 4px 6px rgba(0, 0, 0, 0.3)'
  });

  // Función helper para obtener estilo de botón de dirección
  const getDireccionButtonStyle = (direccion) => ({
    ...styles.joyButton,
    backgroundColor: direccionActiva === direccion ? '#50fa7b' : '#333',
    color: direccionActiva === direccion ? '#000' : 'white',
    transform: direccionActiva === direccion ? 'scale(1.1)' : 'scale(1)',
    boxShadow: direccionActiva === direccion 
      ? '0 0 20px rgba(80, 250, 123, 0.8)' 
      : 'none'
  });

  if (!isLoggedIn) {
    return (
      <div style={styles.loginPage}>
        <div style={styles.loginContainer}>
          <form onSubmit={handleLogin}>
            <h1 style={styles.title}>Control Remoto del Carro</h1>
            <h2 style={styles.subtitle}>Control y Monitoreo</h2>

            <div style={styles.inputGroup}>
              <input
                type="text"
                placeholder="Usuario"
                required
                value={username}
                onChange={(e) => setUsername(e.target.value)}
                disabled={loading}
                style={styles.input}
              />
              <FaUser style={styles.icon} />
            </div>

            <div style={styles.inputGroup}>
              <input
                type={mostrarContrasena ? "text" : "password"}
                placeholder="Contraseña"
                required
                value={password}
                onChange={(e) => setPassword(e.target.value)}
                disabled={loading}
                style={styles.input}
              />
              <span 
                onClick={() => setMostrarContrasena(!mostrarContrasena)}
                style={{ cursor: 'pointer' }}
              >
                {mostrarContrasena ? (
                  <FaEyeSlash style={styles.icon} />
                ) : (
                  <FaEye style={styles.icon} />
                )}
              </span>
            </div>

            {authError && <p style={styles.errorMessage}>{authError}</p>}

            <button type="submit" disabled={loading} style={styles.button}>
              {loading ? "Iniciando sesión..." : "Ingresar"}
            </button>
          
          </form>
        </div>
      </div>
    );
  }

  return (
    <div style={styles.app}>
      <div style={styles.header}>
        <h1 style={styles.headerTitle}>Control del Carrito</h1>
        <button onClick={handleLogout} style={styles.logoutButton}>
          <FaSignOutAlt /> Cerrar Sesión
        </button>
      </div>

      <div style={styles.dashboard}>
        <div style={styles.leftPanel}>
          <h2 style={styles.sectionTitle}><FaLightbulb /> Luces</h2>
          <div style={styles.buttonGrid}>
            <button 
              style={getLuzButtonStyle("delantera")} 
              onClick={() => manejarLuz("delantera")}
            >
              Delantera
            </button>
            <button 
              style={getLuzButtonStyle("traseras")} 
              onClick={() => manejarLuz("traseras")}
            >
              Traseras
            </button>
            <button 
              style={getLuzButtonStyle("izquierda")} 
              onClick={() => manejarLuz("izquierda")}
            >
              Direccional Izquierda
            </button>
            <button 
              style={getLuzButtonStyle("derecha")} 
              onClick={() => manejarLuz("derecha")}
            >
              Direccional Derecha
            </button>
          </div>

          <h3 style={{ color: "white", marginTop: 20 }}>Movimiento</h3>
          <div style={styles.joystick}>
            <div>
              <button 
                style={getDireccionButtonStyle("adelante_izquierda")} 
                onClick={() => moverCarroConDireccion("adelante_izquierda")}
              >
                ↖️
              </button>
              <button 
                style={getDireccionButtonStyle("adelante")} 
                onClick={() => moverCarroConDireccion("adelante")}
              >
                ⬆️
              </button>
              <button 
                style={getDireccionButtonStyle("adelante_derecha")} 
                onClick={() => moverCarroConDireccion("adelante_derecha")}
              >
                ↗️
              </button>
            </div>
            <div>
              <button 
                style={getDireccionButtonStyle("atras_izquierda")} 
                onClick={() => moverCarroConDireccion("atras_izquierda")}
              >
                ↙️
              </button>
              <button 
                style={getDireccionButtonStyle("atras")} 
                onClick={() => moverCarroConDireccion("atras")}
              >
                ⬇️
              </button>
              <button 
                style={getDireccionButtonStyle("atras_derecha")} 
                onClick={() => moverCarroConDireccion("atras_derecha")}
              >
                ↘️
              </button>
            </div>
          </div>
          <button 
            style={{
              ...styles.speedButton,
              background: '#ff5555',
              color: 'white',
              marginBottom: '8px'
            }}
            onClick={detenerCarro}
          >
            🛑 Detener
          </button>


          <button style={styles.speedButton} onClick={cambiarVelocidad}>
            ⚙️ Velocidad: {velocidad}%
          </button>
        </div>

        <div style={styles.centerPanel}>
          <h2 style={styles.sectionTitle}>📊 Estado</h2>
          <div style={styles.statusBox}>
            <h3 style={{ marginBottom: "10px" }}>💡 Estado de Luces</h3>
            <p>🚘 Delantera: <b style={{ color: estadoCarro.luces.delantera ? "#50fa7b" : "#ff5555" }}>
              {estadoCarro.luces.delantera ? "Encendida" : "Apagada"}
            </b></p>
            <p>🔊 Traseras (buzzer): <b style={{ color: estadoCarro.luces.traseras ? "#50fa7b" : "#ff5555" }}>
              {estadoCarro.luces.traseras ? "Encendida" : "Apagada"}
            </b></p>
            <p>⬅️ Izquierda: <b style={{ color: estadoCarro.luces.izquierda ? "#50fa7b" : "#ff5555" }}>
              {estadoCarro.luces.izquierda ? "Encendida" : "Apagada"}
            </b></p>
            <p>➡️ Derecha: <b style={{ color: estadoCarro.luces.derecha ? "#50fa7b" : "#ff5555" }}>
              {estadoCarro.luces.derecha ? "Encendida" : "Apagada"}
            </b></p>

            <hr style={{ border: "1px solid rgba(255,255,255,0.2)", margin: "10px 0" }} />

            <p>🧭 Dirección: <b>{estadoCarro.direccion}</b></p>
            <p>⚙️ Velocidad actual: <b>{velocidad}%</b></p>
          </div>

        </div>

        <div style={styles.rightPanel}>
          <h2 style={styles.sectionTitle}><FaCamera /> Cámara</h2>
          <VideoStream apiBase={API_BASE} credentials={credentials} />
          <button 
            onClick={tomarFoto} 
            disabled={cargandoFoto}
            style={{
              ...styles.cameraButton,
              opacity: cargandoFoto ? 0.6 : 1,
              marginTop: "15px"
            }}
          >
            {cargandoFoto ? "📸 Capturando..." : "📸 Tomar Foto"}
          </button>

          {foto === "error" && (
            <p style={styles.errorMessage}>❌ Error al capturar la foto</p>
          )}
          
          {foto && foto !== "error" && (
            <div style={styles.imagePreview}>
              <img 
                src={foto} 
                alt={camara === "jardin" ? "Vista del jardín" : "Vista frontal"} 
                style={styles.image}
              />
              <p style={styles.timestamp}>
                📅 {new Date().toLocaleString()}
              </p>
            </div>
          )}
        </div>
      </div>
    </div>
  );
}

const styles = {
  loginPage: {
    minHeight: '100vh',
    background: 'linear-gradient(135deg, #667eea 0%, #764ba2 100%)',
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center',
    fontFamily: 'Arial, sans-serif',
    padding: '20px'
  },
  loginContainer: {
    margin: 'auto',
    width: '420px',
    maxWidth: '100%',
    backgroundColor: 'rgba(0, 0, 0, 0.55)',
    border: '2px solid rgba(25, 255, 255, 0.2)',
    color: 'white',
    borderRadius: '10px',
    padding: '30px 40px',
    textAlign: 'center',
    boxSizing: 'border-box',
    backdropFilter: 'blur(10px)'
  },
  title: {
    fontSize: '36px',
    margin: '0 0 10px 0'
  },
  subtitle: {
    fontSize: '18px',
    marginTop: '5px',
    marginBottom: '30px',
    opacity: 0.9
  },
  inputGroup: {
    position: 'relative',
    width: '100%',
    height: '45px',
    margin: '20px 0'
  },
  input: {
    width: '100%',
    height: '100%',
    background: 'transparent',
    outline: 'none',
    border: '2px solid white',
    borderRadius: '40px',
    fontSize: '16px',
    color: 'white',
    padding: '0 20px 0 45px',
    boxSizing: 'border-box'
  },
  icon: {
    position: 'absolute',
    left: '15px',
    top: '50%',
    transform: 'translateY(-50%)',
    fontSize: '20px',
    color: 'white',
    cursor: 'pointer'
  },
  errorMessage: {
    color: 'rgb(255, 100, 100)',
    fontSize: '14px',
    marginTop: '10px',
    backgroundColor: 'rgba(255, 0, 0, 0.1)',
    padding: '8px',
    borderRadius: '5px'
  },
  button: {
    width: '60%',
    height: '45px',
    backgroundColor: 'white',
    border: 'none',
    outline: 'none',
    borderRadius: '40px',
    boxShadow: '0 0 10px rgba(0, 0, 0, 0.1)',
    cursor: 'pointer',
    fontSize: '16px',
    fontWeight: '700',
    color: '#333',
    marginTop: '15px',
    transition: '0.3s ease'
  },
  app: {
    minHeight: '100vh',
    background: 'linear-gradient(135deg, #667eea 0%, #764ba2 100%)',
    padding: '20px',
    fontFamily: 'Arial, sans-serif'
  },
  header: {
    display: 'flex',
    justifyContent: 'space-between',
    alignItems: 'center',
    backgroundColor: 'rgba(0, 0, 0, 0.55)',
    padding: '20px 30px',
    borderRadius: '10px',
    marginBottom: '20px',
    backdropFilter: 'blur(10px)'
  },
  headerTitle: {
    color: 'white',
    margin: 0,
    fontSize: '28px'
  },
  logoutButton: {
    backgroundColor: '#ff79c6',
    color: 'white',
    border: 'none',
    padding: '10px 20px',
    borderRadius: '25px',
    cursor: 'pointer',
    fontSize: '16px',
    fontWeight: 'bold',
    display: 'flex',
    alignItems: 'center',
    gap: '8px',
    transition: '0.3s ease'
  },
  dashboard: {
    display: 'grid',
    gridTemplateColumns: 'repeat(auto-fit, minmax(300px, 1fr))',
    gap: '20px'
  },
  sectionTitle: {
    color: 'white',
    marginTop: 0,
    marginBottom: '20px',
    fontSize: '22px',
    display: 'flex',
    alignItems: 'center',
    gap: '10px'
  },
  buttonGrid: {
    display: 'grid',
    gridTemplateColumns: 'repeat(auto-fit, minmax(140px, 1fr))',
    gap: '10px'
  },
  controlButton: {
    padding: '15px',
    border: 'none',
    borderRadius: '10px',
    cursor: 'pointer',
    fontSize: '14px',
    transition: 'all 0.3s ease',
  },
  cameraButton: {
    width: '100%',
    padding: '15px',
    backgroundColor: '#50fa7b',
    border: 'none',
    borderRadius: '10px',
    cursor: 'pointer',
    fontSize: '16px',
    fontWeight: 'bold',
    color: '#333',
    marginBottom: '15px',
    transition: '0.3s ease'
  },
  imagePreview: {
    marginTop: '15px',
    backgroundColor: 'rgba(0, 0, 0, 0.3)',
    padding: '15px',
    borderRadius: '10px'
  },
  image: {
    width: '100%',
    borderRadius: '8px',
    marginBottom: '10px'
  },
  timestamp: {
    color: 'white',
    textAlign: 'center',
    margin: 0,
    fontSize: '14px',
    opacity: 0.8
  },
  leftPanel: {
    backgroundColor: 'rgba(0,0,0,0.55)',
    padding: '20px',
    borderRadius: '10px',
    display: 'flex',
    flexDirection: 'column',
    alignItems: 'center',
    gap: '10px'
  },
  centerPanel: {
    backgroundColor: 'rgba(0,0,0,0.55)',
    padding: '20px',
    borderRadius: '10px',
    textAlign: 'center',
    color: 'white'
  },
  rightPanel: {
    backgroundColor: 'rgba(0,0,0,0.55)',
    padding: '20px',
    borderRadius: '10px'
  },
  joystick: {
    display: 'flex',
    flexDirection: 'column',
    alignItems: 'center',
    gap: '5px',
    marginTop: '10px'
  },
  joyButton: {
    width: '60px',
    height: '60px',
    borderRadius: '12px',
    border: 'none',
    fontSize: '24px',
    margin: '3px',
    cursor: 'pointer',
    transition: 'all 0.2s ease'
  },
  speedButton: {
    marginTop: '15px',
    padding: '12px 20px',
    borderRadius: '10px',
    border: 'none',
    background: '#50fa7b',
    color: '#000',
    fontWeight: 'bold',
    cursor: 'pointer'
  },
  statusBox: {
    backgroundColor: 'rgba(255,255,255,0.1)',
    borderRadius: '10px',
    padding: '15px',
    color: 'white',
    lineHeight: '1.8em'
  }
};

export default App;