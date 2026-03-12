import { CzmlDataSource, type Viewer } from "cesium";
import type { LiveTracker } from "./live-tracker.js";
import type { CzmlLivePacket } from "./types.js";

export class CzmlHandler {
  private viewer: Viewer;
  private dataSource: CzmlDataSource;
  private initialized = false;
  private liveTracker: LiveTracker | null = null;

  constructor(viewer: Viewer) {
    this.viewer = viewer;
    this.dataSource = new CzmlDataSource("SensorData");
    viewer.dataSources.add(this.dataSource);
  }

  setLiveTracker(tracker: LiveTracker): void {
    this.liveTracker = tracker;
  }

  async handleBulkLoad(czmlJson: string): Promise<void> {
    const czml = JSON.parse(czmlJson) as object[];

    // Create a fresh data source — avoids stale polyline primitive state
    const oldDs = this.dataSource;
    const newDs = new CzmlDataSource("SensorData");
    await newDs.load(czml);

    // Swap: add new first (minimizes visual gap), then remove old
    this.viewer.dataSources.add(newDs);
    this.viewer.dataSources.remove(oldDs, true);
    this.dataSource = newDs;
    this.initialized = true;
  }

  async handleLivePacket(czmlJson: string): Promise<void> {
    const czml = JSON.parse(czmlJson) as CzmlLivePacket[];
    if (!this.initialized) {
      // CzmlDataSource.process() requires a document packet first.
      // Prepend one so the first live packet doesn't throw.
      czml.unshift({ id: "document", version: "1.0" });
      await this.dataSource.load(czml);
      this.initialized = true;
    } else {
      await this.dataSource.process(czml);
    }

    // Feed position samples to the live path tracker
    if (this.liveTracker) {
      for (const packet of czml) {
        if (
          typeof packet.id === "string" &&
          packet.id.startsWith("live-") &&
          packet.position?.cartographicDegrees
        ) {
          const timestampMs = parseInt(packet.id.substring(5), 10);
          const [lon, lat, alt] = packet.position.cartographicDegrees;
          this.liveTracker.addSample(timestampMs, lon, lat, alt);
        }
      }
    }
  }
}
