export interface Signal<T extends (...args: never[]) => void> {
  connect(callback: T): void;
  disconnect(callback: T): void;
}

export interface CesiumBridgeProxy {
  readonly cesiumToken: string;
  liveMode: boolean;
  readonly pendingRequestId: number;
  readonly jsReady: boolean;
  windowMinutes: number;

  loadRange(startMsecs: number, endMsecs: number, requestId: number): void;
  getThresholdConfig(callback: (result: string) => void): void;
  setJsReady(ready: boolean): void;

  czmlReady: Signal<(czmlJson: string, requestId: number) => void>;
  czmlPacket: Signal<(czmlJson: string) => void>;
  liveModeChanged: Signal<(liveMode: boolean) => void>;
  cesiumTokenChanged: Signal<() => void>;
  pendingRequestIdChanged: Signal<(id: number) => void>;
  windowMinutesChanged: Signal<(minutes: number) => void>;
  thresholdsChanged: Signal<(config: object) => void>;
  jsReadyChanged: Signal<() => void>;
}

export interface CzmlLivePacket {
  id: string;
  position?: {
    cartographicDegrees: [number, number, number]; // [lon, lat, alt]
  };
  [key: string]: unknown;
}

declare global {
  const qt: {
    webChannelTransport: unknown;
  };
}
