
const d = document;
const api = 'https://humble-space-goldfish-pqjwvw4jxxx299pj-5000.app.github.dev/';

var ctx = document.querySelector("#chart").getContext('2d');
var chartMqtt = new Chart(ctx, {
    type: 'line',
    data: {
        labels: [],
        datasets: [{
            label: 'Humedad',
            data: [67],
            fill: false,
            borderColor: 'rgb(75, 192, 192)',
            tension: 0.1
        },
        {
            label: 'Temperatura',
            data: [21],
            fill: false,
            borderColor: 'rgb(255,82,82)',
            tension: 0.1
        }],
    },    
    options: {
        responsive: true,
        width: 400,
        height: 200 
    }
});

const extractData = (payload) => {
    console.log(payload);
    const datos = payload.split(":");
    const datosFormat = datos.map(el => {
      return el.trim();
    });
    return datosFormat;
}

const render = (data) => {
    if (data && data.length > 0) {
        const ultimoElemento = data[data.length - 1];
        let currentTime = moment().format('HH:mm:ss');
        const formatData = extractData(ultimoElemento.payload);
        document.querySelector('#data-container').insertAdjacentHTML("afterbegin",`
            <li class="list-group-item">
            <span class="badge text-bg-success text-wrap rounded-0">${currentTime}</span>
            ${ultimoElemento.payload}
            </li>
        `);
        addData(chartMqtt, currentTime, formatData);
    }
}

const fetchHum = () => {
    fetch(`${api}/humedad`)
        .then(response => response.json())
        .then(data => render(data))
        .catch(error => console.error('Error al obtener los datos:', error));
}

const fetchTemp = () =>  {
    fetch(`${api}/temperatura`)
        .then(response => response.json())
        .then(data => render(data))
        .catch(error => console.error('Error al obtener los datos:', error));
}

const addData = (chart, label, data) => {
    let timeStamps = chart.data.labels.slice(-9);
    timeStamps.push(label);
    chart.data.labels = timeStamps;
    if(data[0].toLowerCase() === 'humedad'){
        chart.data.datasets[0].data.push(data[1]);
    }
    if(data[0].toLowerCase() === 'temperatura'){
        chart.data.datasets[1].data.push(data[1]);
    }
    chart.update();
}


d.addEventListener("DOMContentLoaded", (e)=>{
    setInterval(fetchHum, 5000);
    setInterval(fetchTemp, 5000);
});
