import {
  type Viewer,
  Entity,
  SampledPositionProperty,
  JulianDate,
  TimeInterval,
  Cartesian3,
  ExtrapolationType,
  Color,
  PolylineGlowMaterialProperty,
} from "cesium";

export class LiveTracker {
  private static readonly EPOCH = JulianDate.fromDate(new Date(0));
  private static readonly PRUNE_MARGIN = 5; // seconds beyond trailTime

  private viewer: Viewer;
  private entity: Entity | null = null;
  private positionProperty: SampledPositionProperty | null = null;
  private readonly trailTime: number;

  constructor(viewer: Viewer, trailTime: number = 30) {
    this.viewer = viewer;
    this.trailTime = trailTime;
  }

  start(): void {
    this.stop();

    this.positionProperty = new SampledPositionProperty();
    this.positionProperty.forwardExtrapolationType = ExtrapolationType.HOLD;
    this.positionProperty.forwardExtrapolationDuration = 60;

    this.entity = this.viewer.entities.add({
      position: this.positionProperty,
      path: {
        leadTime: 0,
        trailTime: this.trailTime,
        width: 3,
        resolution: 1,
        material: new PolylineGlowMaterialProperty({
          glowPower: 0.2,
          color: Color.fromCssColorString("#2196F3"),
        }),
      },
      point: {
        pixelSize: 14,
        color: Color.CYAN,
      },
    });
  }

  stop(): void {
    if (this.entity) {
      this.viewer.entities.remove(this.entity);
      this.entity = null;
      this.positionProperty = null;
    }
  }

  addSample(timestampMs: number, lon: number, lat: number, alt: number): void {
    if (!this.positionProperty) return;
    const time = JulianDate.fromDate(new Date(timestampMs));
    const position = Cartesian3.fromDegrees(lon, lat, alt);
    this.positionProperty.addSample(time, position);

    // Prune samples older than the visible trail window
    const cutoff = JulianDate.addSeconds(
      time,
      -(this.trailTime + LiveTracker.PRUNE_MARGIN),
      new JulianDate(),
    );
    this.positionProperty.removeSamples(
      new TimeInterval({ start: LiveTracker.EPOCH, stop: cutoff }),
    );
  }

  isActive(): boolean {
    return this.entity !== null;
  }
}
