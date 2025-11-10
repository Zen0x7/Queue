import type { AvailableRouterMethod  } from 'nitropack/types';

export class Api {
    static async call(method: AvailableRouterMethod<string>, path: string, payload?: string) {
        try {
            const data = await $fetch(path, {
                method: method,
                body: payload,
                baseURL: "http://127.0.0.1:9000",
            });

            return data;
        } catch (error) {
            console.error(error);
            throw error;
        }
    }
}

